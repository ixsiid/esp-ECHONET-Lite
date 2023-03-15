#include <esp_log.h>
#include <esp_netif_ip_addr.h>
#include "evps-object.hpp"

const char EVPS::TAG[] = "EL EVPS";

EVPS::EVPS(uint8_t instance) : ELObject(instance, EVPS::class_u16), evps{} {
	update_mode_cb = nullptr;

	//// スーパークラス
	// 設置場所
	evps[0x81] = new uint8_t[0x02]{0x01, 0b01101101};
	// 規格Version情報
	evps[0x82] = new uint8_t[0x05]{0x04, 0x00, 0x00, 'J', 0x01};  // Qでは登録不可
	// 異常発生状態
	evps[0x88] = new uint8_t[0x02]{0x01, 0x42};	// 異常なし
	// メーカコード
	evps[0x8a] = maker_code;
	// 状態変更通知アナウンスマップ
	evps[0x9d] = new uint8_t[0x07]{0x06, 0x05, 0x80, 0xc7, 0xda, 0xdc, 0xdd};
	// Setプロパティマップ
	evps[0x9e] = new uint8_t[0x04]{0x03, 0x02, 0xcd, 0xda};
	// Getプロパティマップ
	evps[0x9f] = new uint8_t[0x12]{0x11, 0x1f,
							 // fedcba98
							 0b00110001,
							 0b00000001,
							 0b01010001,
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

	evps[0x80] = new uint8_t[0x02]{0x01, 0x30};	// !!

	evps[0xc0] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x42, 0x20};  // 不可応答で可
	evps[0xc2] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x21, 0x10};  // 0xc4で代替、不可応答でも可, 応答が52と72が混在した時の処理が手間のため、一旦両方返却する
	evps[0xc4] = new uint8_t[0x02]{0x01, 0x32};				   // 0xc2で代替、不可応答でも可
	evps[0xc5] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x0b, 0xb8};
	evps[0xc6] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};
	evps[0xc7] = new uint8_t[0x02]{0x01, 0x41};	// 接続状態：充電可・放電不可

	evps[0xc8] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x0b, 0xb8, 0x00, 0x00, 0x0b, 0xb8};  // !!
	evps[0xc9] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // !!
	evps[0xca] = new uint8_t[0x05]{0x04, 0x01, 0x2c, 0x01, 0x2c};					  // !!
	evps[0xcb] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};					  // !!
	evps[0xcc] = new uint8_t[0x02]{0x01, 0x22};									  // DC_AA (CHaDeMo) DCタイプじゃないと登録不可
	evps[0xcd] = new uint8_t[0x02]{0x01, 0x41};									  // SET
	evps[0xce] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0xea, 0x60};					  // 不可応答で可
	evps[0xcf] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x75, 0x30};					  // 不可応答で可

	evps[0xd0] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0xea, 0x60};  // 不可応答で可

	evps[0xd3] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};  // 瞬時値
	evps[0xd6] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};  // 積算放電電力量
	evps[0xd8] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};  // 積算充電電力量

	evps[0xda] = new uint8_t[0x02]{0x01, 0x44};	// GET/SET 運転モード：待機
	evps[0xdc] = new uint8_t[0x02]{0x01, 0x01};	// !!
	evps[0xdd] = new uint8_t[0x02]{0x01, 0x00};	// !!

	evps[0xe2] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x75, 0x30};  // 0xe4で代替、不可応答で可
	evps[0xe4] = new uint8_t[0x02]{0x01, 0x32};				   // 不可応答で可
	evps[0xe6] = new uint8_t[0x0b]{0x0a, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
};

uint8_t EVPS::set(uint8_t* epcs, uint8_t count) {
	// ESP_LOGI(TAG, "EVPS: get %d", count);
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

		if (evps[epc] == nullptr) return 0;

		switch (epc) {
			case 0xda:  // 運転モード
				if (update_mode_cb) {
					Mode new_mode = update_mode_cb((Mode)evps[epc][1], (Mode)*t);
					if (new_mode == Mode::Unacceptable) return 0;

					evps[epc][1] = (uint8_t)new_mode;
				} else {
					evps[epc][1] = *t;
				}
				break;
			default:
				memcpy(&(evps[epc][1]), t, len);
				break;
		}

		if (len > 0) {
			ESP_LOG_BUFFER_HEXDUMP(TAG, t, len, ESP_LOG_INFO);
			t += len;
		}

		// メモリ更新後の値を返却する
		*n = epc;
		n++;
		memcpy(n, evps[epc], evps[epc][0] + 1);
		n += evps[epc][0] + 1;
	}

	buffer_length = sizeof(elpacket_t) + (n - epc_start);

	return res_count;
}

uint8_t EVPS::get(uint8_t* epcs, uint8_t count) {
	ESP_LOGI(TAG, "EVPS: get %d", count);
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

		if (evps[epc] == nullptr) return 0;

		if (len > 0) {
			ESP_LOG_BUFFER_HEXDUMP(TAG, t, len, ESP_LOG_INFO);
			t += len;
		}

		*n = epc;
		n++;
		memcpy(n, evps[epc], evps[epc][0] + 1);
		n += evps[epc][0] + 1;
	}

	buffer_length = sizeof(elpacket_t) + (n - epc_start);

	return res_count;
};

void EVPS::set_update_mode_cb(update_mode_cb_t cb) {
	update_mode_cb = cb;
};

void EVPS::notify_mode() {
	p->epc_count  = 1;
	p->esv	    = 0x74;  // ESV_INFC
	epc_start[0]  = 0xda;
	epc_start[1]  = evps[0xda][0];
	epc_start[2]  = evps[0xda][1];
	buffer_length = sizeof(elpacket_t) + 3;
};
