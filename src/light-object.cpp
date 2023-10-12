#include <esp_log.h>
#include "light-object.hpp"

const char Light::TAG[] = "EL Light";

Light::Light(uint8_t instance) : ELObject(instance, Light::class_u16) {
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
	props[0x9e] = new uint8_t[0x04]{0x03, 0x02, 0x80, 0xb6};
	// Getプロパティマップ
	props[0x9f] = new uint8_t[0x0b]{0x0a, 0x09, 0x81, 0x82, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0x80, 0xb6};

	//// 固有クラス
	// 動作状態: ON 0x30, OFF 0x31
	props[0x80] = new uint8_t[0x02]{0x01, 0x31};
	// モード: 自動0x41, 通常灯0x42, 常夜灯0x43, カラー灯0x45
	props[0xb6] = new uint8_t[0x02]{0x01, 0x42};
};