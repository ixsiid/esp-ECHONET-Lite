#include <esp_log.h>
#include "instant-heater-object.hpp"

const char InstantHeater::TAG[] = "EL Instant Heater";

InstantHeater::InstantHeater(uint8_t instance) : ELObject(instance, InstantHeater::class_u16) {
	//// スーパークラス
	// 設置場所
	props[0x81] = new uint8_t[0x02]{0x01, 0b01111101};
	// 規格Version情報: Release Q
	props[0x82] = new uint8_t[0x05]{0x04, 0x00, 0x00, 'Q', 0x01};
	// 異常発生状態: 異常なし0x42
	props[0x88] = new uint8_t[0x02]{0x01, 0x42};
	// メーカコード
	props[0x8a] = maker_code;
	// 状態変更通知アナウンスマップ
	props[0x9d] = new uint8_t[0x03]{0x02, 0x01, 0x80};
	// Setプロパティマップ
	props[0x9e] = new uint8_t[0x05]{0x04, 0x03, 0x80, 0xe1, 0xe3};
	// Getプロパティマップ
	props[0x9f] = new uint8_t[0x0e]{0x0d, 0x0c, 0x81, 0x82, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0x80, 0xd0, 0xe1, 0xe2, 0xe3};

	//// 固有クラス
	// 動作状態: ON 0x30, OFF 0x31
	props[0x80] = new uint8_t[0x02]{0x01, 0x30};
	// 給湯器燃焼状態 燃焼状態有0x41, 燃焼状態無0x42
	props[0xd0] = new uint8_t[0x02]{0x01, 0x42};
	// 風呂温度設定値 0x00 - 0x64 (0 - 100℃)
	props[0xe1] = new uint8_t[0x02]{0x01, 0x24};
	// 風呂給湯器燃焼状態 燃焼状態有0x41, 燃焼状態無0x42
	props[0xe2] = new uint8_t[0x02]{0x01, 0x42};
	// 風呂自動モード 自動入0x41, 自動解除0x42
	props[0xe3] = new uint8_t[0x02]{0x01, 0x42};
};
