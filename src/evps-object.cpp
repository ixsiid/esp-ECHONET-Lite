#include <esp_log.h>
#include <esp_netif_ip_addr.h>
#include "evps-object.hpp"

const char EVPS::TAG[] = "EL EVPS";

EVPS::EVPS(uint8_t instance) : ELObject(instance, EVPS::class_u16) {
	//// スーパークラス
	// 設置場所
	props[0x81] = new uint8_t[0x02]{0x01, 0b01101101};
	// 規格Version情報
	props[0x82] = new uint8_t[0x05]{0x04, 0x00, 0x00, 'Q', 0x01};  // Qでは登録不可
	// 異常発生状態
	props[0x88] = new uint8_t[0x02]{0x01, 0x42};	// 異常なし
	// メーカコード
	props[0x8a] = maker_code;
	// 状態変更通知アナウンスマップ
	props[0x9d] = new uint8_t[0x07]{0x06, 0x05, 0x80, 0xc7, 0xda, 0xdc, 0xdd};
	// Setプロパティマップ
	props[0x9e] = new uint8_t[0x04]{0x03, 0x02, 0xcd, 0xda};
	// Getプロパティマップ
	props[0x9f] = new uint8_t[0x12]{0x11, 0x1d,
							 //fedcba98
							 0b00110001,
							 0b00000001,
							 0b00000001,
							 0b00100000,
							 0b01010000,
							 0b00010000,  // 0x_5
							 0b01110000,
							 0b00010000,
							 0b00110001,
							 0b00010000,
							 0b00110001,  // 0x_a
							 0b00010000,
							 0b00110000,
							 0b00100010,
							 0b00010010,
							 0b00010010};

	props[0x80] = new uint8_t[0x02]{0x01, 0x30};	// !!

	props[0xc0] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x42, 0x20};  // 不可応答で可
	// props[0xc2] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x21, 0x10};  // 0xc4で代替
	props[0xc4] = new uint8_t[0x02]{0x01, 0x00}; // 放電可能容量, 0%
	props[0xc5] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x0b, 0xb8}; // 定格充電能力 [W], 3000
	props[0xc6] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00}; // 定格放電能力 [W], 0
	props[0xc7] = new uint8_t[0x02]{0x01, 0x41};	// 接続状態：充電可・放電不可

	props[0xc8] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x0b, 0xb8, 0x00, 0x00, 0x0b, 0xb8}; // !!
	props[0xc9] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // !!
	props[0xca] = new uint8_t[0x05]{0x04, 0x01, 0x2c, 0x01, 0x2c};					  // !!
	props[0xcb] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};					  // !!
	props[0xcc] = new uint8_t[0x02]{0x01, static_cast<uint8_t>(Type::DC_AA)};			  // DC_AA (CHaDeMo) DCタイプじゃないと登録不可
	props[0xcd] = new uint8_t[0x02]{0x01, 0x00};									  // SET
	props[0xce] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0xea, 0x60};					  // 充電可能容量 [Wh]
	props[0xcf] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x75, 0x30};					  // 充電可能残容量 [Wh]

	props[0xd0] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0xea, 0x60};  // 車載電池容量[Wh]

	props[0xd3] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};  // 瞬時値
	props[0xd6] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};  // 積算放電電力量
	props[0xd8] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};  // 積算充電電力量

	props[0xda] = new uint8_t[0x02]{0x01, 0x44};	// GET/SET 運転モード：待機
	props[0xdc] = new uint8_t[0x02]{0x01, 0x01};	// !!
	props[0xdd] = new uint8_t[0x02]{0x01, 0x00};	// !!

	// props[0xe2] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x75, 0x30};  // 0xe4で代替
	props[0xe4] = new uint8_t[0x02]{0x01, 0x00}; // 充電%, 0% (不可応答で可)
	props[0xe6] = new uint8_t[0x0a]{0x09, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

	timer = xTaskGetTickCount();
	accumulation_charging_watt_tick = 0;
	accumulation_discharging_watt_tick = 0;
};

void EVPS::set_mode(Mode mode) {
	props[0xda][1] = static_cast<uint8_t>(mode);
};

void EVPS::update() {
	// 積算値を更新する
	portTickType t = xTaskGetTickCount();
	int watt		= (props[0xd3][1] << 24 |
			        props[0xd3][2] << 16 |
			        props[0xd3][3] << 8 |
			        props[0xd3][4] << 0);
	int watt_tick	= watt * (t - timer);
	timer		= t;

	if (watt > 0) {
		accumulation_charging_watt_tick += watt_tick;
		uint32_t wh = accumulation_charging_watt_tick / portTICK_PERIOD_MS / 3600 / 10;

		props[0xd8][1] = wh >> 24;
		props[0xd8][2] = wh >> 16;
		props[0xd8][3] = wh >> 8;
		props[0xd8][4] = wh >> 0;
	} else if (watt < 0) {
		accumulation_discharging_watt_tick += watt_tick;
		uint32_t wh = accumulation_discharging_watt_tick / portTICK_PERIOD_MS / 3600 / 10;

		props[0xd6][1] = wh >> 24;
		props[0xd6][2] = wh >> 16;
		props[0xd6][3] = wh >> 8;
		props[0xd6][4] = wh >> 0;
	}
};

void EVPS::update_input_output(int watt) {
	update();
	if (watt > 0) {
		props[0xda][1] = static_cast<uint8_t>(Mode::Charge);
	} else if (watt < 0) {
		props[0xda][1] = static_cast<uint8_t>(Mode::Discharge);
	} else {
		props[0xda][1] = static_cast<uint8_t>(Mode::Stop);
	}

	props[0xd3][1] = watt >> 24;
	props[0xd3][2] = watt >> 16;
	props[0xd3][3] = watt >> 8;
	props[0xd3][4] = watt >> 0;
};

void EVPS::update_remain_battery_ratio(uint8_t ratio) {
	props[0xe4][1] = ratio;
};
