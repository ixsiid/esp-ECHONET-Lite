#include <esp_log.h>
#include <esp_netif_ip_addr.h>
#include "power-distribution-object.hpp"

const char PowerDistribution::TAG[] = "ELPDist";

PowerDistribution::PowerDistribution(uint8_t instance, uint8_t ch_count) : ELObject(instance, PowerDistribution::class_u16) {
	//// スーパークラス
	// 設置場所
	props[0x81] = new uint8_t[0x02]{0x01, 0b01111101};  // その他
	// 規格Version情報
	props[0x82] = new uint8_t[0x05]{0x04, 0x00, 0x00, 'Q', 0x01};  // Qでは登録不可
	// 異常発生状態
	props[0x88] = new uint8_t[0x02]{0x01, 0x42};	 // 異常なし
	// メーカコード
	props[0x8a] = maker_code;
	// 状態変更通知アナウンスマップ
	props[0x9d] = new uint8_t[0x03]{0x02, 0x01, 0x80};
	// Setプロパティマップ
	props[0x9e] = new uint8_t[0x02]{0x01, 0x00};
	// Getプロパティマップ
	props[0x9f] = new uint8_t[0x08 + ch_count]{
	    (uint8_t)(0x07 + ch_count), (uint8_t)(0x06 + ch_count),
	    0x80, 0xc0, 0xc1, 0xc2, 0xb1, 0xb3};

	// 固有クラス
	props[0x80] = new uint8_t[0x02]{0x01, 0x31};
	props[0xc0] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};
	props[0xc1] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};
	props[0xc2] = new uint8_t[0x02]{0x01, 0x03};

	props[0xb1] = new uint8_t[0x02]{0x01, ch_count};
	props[0xb3] = new uint8_t[0x07]{0x06, 0x01, ch_count, 0x00, 0x00, 0x00, 0x00};

	// ch_countが7以上だとGETプロパティマップがあふれる
	measures		= new measure_t[ch_count]();
	this->ch_count = ch_count;
	portTickType t = xTaskGetTickCount();

	for (int i = 0; i < ch_count; i++) {
		measures[i].tick	= t;
		measures[i].voltage = 100; // 一旦100V固定

		props[0x9f][0x08 + i] = 0xd0 + i;

		props[0xd0 + i] = new uint8_t[0x09]{0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	}

	//	83, 97, 98, b0-be, c0-c8, d0-df, e0-ef
	//	b1, b3, c0, c1, c2, d0 -df, e0-ef
};

void PowerDistribution::update() {
	uint32_t wh = 0;
	for (int i = 0; i < ch_count; i++) {
		update(i);

		wh += (props[0xd0 + i][1] << 24 |
			  props[0xd0 + i][2] << 16 |
			  props[0xd0 + i][3] << 8 |
			  props[0xd0 + i][4] << 0);
	}

	// 全合計
	props[0xc0][1] = wh >> 24;
	props[0xc0][2] = wh >> 16;
	props[0xc0][3] = wh >> 8;
	props[0xc0][4] = wh >> 0;

	// 片方向チャネル合計
	// 本来は0xB2で取得範囲指定ができるが未実装
	props[0xb3][3] = wh >> 24;
	props[0xb3][4] = wh >> 16;
	props[0xb3][5] = wh >> 8;
	props[0xb3][6] = wh >> 0;
};

void PowerDistribution::update(uint8_t ch) {
	portTickType t = xTaskGetTickCount();

	measures[ch].watt_tick += measures[ch].ampere * measures[ch].voltage * (t - measures[ch].tick);
	measures[ch].tick = t;

	uint32_t wh = measures[ch].watt_tick / portTICK_PERIOD_MS / 3600 / 10;  // ECHONET Liteプロパティでのアンペアの単位指定が0.1Aだから最後に10で割る

	uint8_t* prop = props[0xd0 + ch];

	prop[1] = wh >> 24;
	prop[2] = wh >> 16;
	prop[3] = wh >> 8;
	prop[4] = wh >> 0;
};

void PowerDistribution::set_ampere(uint8_t ch, uint16_t ampere) {
	update(ch);
	measures[ch].ampere = ampere;

	// R相だけ更新、T相は未実装
	props[0xd0 + ch][5] = ampere >> 8;
	props[0xd0 + ch][6] = ampere >> 0;
};
