#include <esp_log.h>
#include <esp_netif_ip_addr.h>
#include "battery-object.hpp"

const char Battery::TAG[] = "Battery";

Battery::Battery(uint8_t instance) : ELObject(instance, Battery::class_u16), battery{} {
	//// スーパークラス
	// 設置場所
	battery[0x81] = new uint8_t[0x02]{0x01, 0b01111101};  // その他
	// 規格Version情報
	battery[0x82] = new uint8_t[0x05]{0x04, 0x00, 0x00, 'Q', 0x01};	// Qでは登録不可
	// 異常発生状態
	battery[0x88] = new uint8_t[0x02]{0x01, 0x42};  // 異常なし
	// メーカコード
	battery[0x8a] = maker_code;
	// 状態変更通知アナウンスマップ
	battery[0x9d] = new uint8_t[0x09]{0x08, 0x07, 0x80, 0xaa, 0xab, 0xc1, 0xc2, 0xcf, 0xda};
	// Setプロパティマップ
	battery[0x9e] = new uint8_t[0x05]{0x04, 0x03, 0xaa, 0xab, 0xda};
	// Getプロパティマップ
	battery[0x9f] = new uint8_t[0x12]{0x11, 0x18,
							    //fedcba98
							    0b00000101,
							    0b00010100,
							    0b01010100,
							    0b00100101,
							    0b00000100,
							    0b00000100,  // 0x_5
							    0b01000000,
							    0b00000010,
							    0b00010110,
							    0b00010100,
							    0b00100100,  // 0x_a
							    0b00100100,
							    0b00000000,
							    0b00000000,
							    0b00000000,
							    0b00010000};

	// 固有クラス

	battery[0x80] = new uint8_t[0x02]{0x01, 0x30};
	battery[0x83] = new uint8_t[0x12]{0x11, 0xfe, maker_code[0], maker_code[1], maker_code[2], 0x0d,
							    0x0c, 0x0b, 0x0a, 0x09,
							    0x08, 0x07, 0x06, 0x05,
							    0x04, 0x03, 0x02, 0x04};

	battery[0x97] = new uint8_t[0x03]{0x02, 0x0a, 0x12};			 // 時刻
	battery[0x98] = new uint8_t[0x05]{0x04, 0x30, 0x01, 0x02, 0x03};	 // 年月日

	battery[0xa0] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x27, 0x10};	 // （空の時）充電可能電力量 kWh, 10kWh
	battery[0xa1] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x27, 0x10};	 // （満充電）放電可能電力量 kWh, 10kWh
	battery[0xa2] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x27, 0x10};	 // （通常時）充電可能電力量 kWh, 10kWh
	battery[0xa3] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x27, 0x10};	 // （通常時）放電可能電力量 kWh, 10kWh
	battery[0xa4] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x27, 0x10};	 // （現時点）充電可能電力量 kWh, 10kWh
	battery[0xa5] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x27, 0x10};	 // （現時点）放電可能電力量 kWh, 10kWh
	battery[0xa8] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};	 // 積算充電電力量 Wh
	battery[0xa9] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};	 // 積算放電電力量 Wh
	battery[0xaa] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};	 // Set/Get 充電電力量（設定値） Wh
	battery[0xab] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};	 // Set/Get 放電電力量（設定値） Wh

	battery[0xc1] = new uint8_t[0x02]{0x01, 0x03};  // 充電方式, 指定電力充電
	battery[0xc2] = new uint8_t[0x02]{0x01, 0x03};  // 放電方式, 指定電力放電

	battery[0xc8] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x17, 0x70};	// 最少充電電力, 最大受電電力 W, 10W - 6kW
	battery[0xc9] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x17, 0x70};	// 最少放電電力, 最大放電電力 W, 10W - 6kW
	battery[0xcf] = new uint8_t[0x02]{0x01, static_cast<uint8_t>(Mode::Stanby)};			// 動作状態, 自動

	battery[0xd3] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};	 // (Optional) 瞬時充放電電力

	battery[0xda] = new uint8_t[0x02]{0x01, static_cast<uint8_t>(Mode::Stanby)};  // Set/Get 運転モード設定、 自動
	battery[0xdb] = new uint8_t[0x02]{0x01, 0x00};						   // 系統連系状態, 系統連系（逆潮流可）

	battery[0xe2] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};	 // 蓄電残量 Wh
	// battery[0xe3] = new uint8_t[0x03]{0x02, 0x00, 0x00};		 // 蓄電残量 0.1Ah
	// battery[0xe4] = new uint8_t[0x02]{0x01, 0x00};				 // 蓄電残量 %

	battery[0xe6] = new uint8_t[0x02]{0x01, static_cast<uint8_t>(Type::Unknown)};  // 電池の種類
};

uint8_t Battery::set(uint8_t* epcs, uint8_t count) {
	p->src_device_class = class_group;
	p->src_device_id	= instance;

	uint8_t* t = epcs;
	uint8_t* n = epc_start;
	uint8_t res_count;

	for (res_count = 0; res_count < count; res_count++) {
		uint8_t epc = t[0];
		uint8_t len = t[1];
		ESP_LOGI(TAG, "EPC 0x%02x [%d]", epc, len);
		t += 2;

		if (battery[epc] == nullptr) return 0;

		switch (epc) {
			default:
				memcpy(&(battery[epc][1]), t, len);
				break;
		}

		if (len > 0) {
			ESP_LOG_BUFFER_HEXDUMP(TAG, t, len, ESP_LOG_INFO);
			t += len;
		}

		// メモリ更新後の値を返却する
		*n = epc;
		n++;
		memcpy(n, battery[epc], battery[epc][0] + 1);
		n += battery[epc][0] + 1;
	}

	buffer_length = sizeof(elpacket_t) + (n - epc_start);

	return res_count;
}

uint8_t Battery::get(uint8_t* epcs, uint8_t count) {
	ESP_LOGI(TAG, "Battery: get %d", count);
	p->src_device_class = class_group;
	p->src_device_id	= instance;

	uint8_t* t = epcs;
	uint8_t* n = epc_start;
	uint8_t res_count;

	for (res_count = 0; res_count < count; res_count++) {
		uint8_t epc = t[0];
		uint8_t len = t[1];
		ESP_LOGD(TAG, "EPC 0x%02x [%d]", epc, len);
		t += 2;

		if (battery[epc] == nullptr) return 0;

		if (len > 0) {
			ESP_LOG_BUFFER_HEXDUMP(TAG, t, len, ESP_LOG_INFO);
			t += len;
		}

		*n = epc;
		n++;
		memcpy(n, battery[epc], battery[epc][0] + 1);
		n += battery[epc][0] + 1;
	}

	buffer_length = sizeof(elpacket_t) + (n - epc_start);

	return res_count;
};

void Battery::update(){
    // 積算値を更新する
};

void Battery::update(int watt) {
	update();
	battery[0xd3][1] = static_cast<uint8_t>(watt >> 24);
	battery[0xd3][2] = static_cast<uint8_t>(watt >> 16);
	battery[0xd3][3] = static_cast<uint8_t>(watt >> 8);
	battery[0xd3][4] = static_cast<uint8_t>(watt >> 0);
};

void Battery::notify_mode() {
	p->epc_count  = 1;
	p->esv	    = 0x74;  // ESV_INFC
	epc_start[0]  = 0xda;
	epc_start[1]  = battery[0xda][0];
	epc_start[2]  = battery[0xda][1];
	buffer_length = sizeof(elpacket_t) + 3;
};
