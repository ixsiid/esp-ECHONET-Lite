#include <esp_log.h>
#include "fuel-cell-object.hpp"

const char FuelCell::TAG[] = "EL Fuel Cell";

FuelCell::FuelCell(uint8_t instance) : ELObject(instance, FuelCell::class_u16) {
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
	props[0x9e] = new uint8_t[0x06]{0x05, 0x04, 0xca, 0xcb, 0xd1, 0xd2};
	// Getプロパティマップ
	props[0x9f] = new uint8_t[0x12]{0x11, 0x12, 
//	                                  fedcba98, 
	                                0b00100001 /* 0x_0 */, 
	                                0b00100001 /* 0x_1 */, 
	                                0b00110001 /* 0x_2 */, 
	                                0b00000000 /* 0x_3 */, 
	                                0b00010000 /* 0x_4 */, 
	                                0b00010000 /* 0x_5 */, 
	                                0b00000000 /* 0x_6 */, 
	                                0b00000000 /* 0x_7 */, 
	                                0b00010001 /* 0x_8 */, 
	                                0b00000000 /* 0x_9 */, 
	                                0b00010001 /* 0x_a */, 
	                                0b00010000 /* 0x_b */, 
	                                0b00000000 /* 0x_c */, 
	                                0b00010010 /* 0x_d */, 
	                                0b00000010 /* 0x_e */, 
	                                0b00000010 /* 0x_f */};

	//// 固有クラス
	// 動作状態: ON 0x30, OFF 0x31
	props[0x80] = new uint8_t[0x02]{0x01, 0x31};
	// 定格発電出力 [W]
	props[0xc2] = new uint8_t[0x03]{0x02, 0x0b, 0xb8};
	// 瞬時発電電力 [W]
	props[0xc4] = new uint8_t[0x03]{0x02, 0x00, 0x00};
	// 積算電力量 [0.001kWh]
	props[0xc5] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};
	// 積算ガス消費量 [0x001m3]
	props[0xc8] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};
	// 発電状態: 発電動作0x41, 発電停止0x42
	props[0xca] = new uint8_t[0x02]{0x01, 0x42};
	// 発電動作状態: 発電中0x41, 停止中0x42, 起動中0x43, 停止動作中0x44, アイドル中0x45
	props[0xcb] = new uint8_t[0x02]{0x01, 0x42};
	// 宅内の積算消費電力量 [0x001kWh]
	props[0xcd] = new uint8_t[0x05]{0x04, 0x00, 0x00, 0x00, 0x00};
	// 系統連系状態: 逆潮流可0x00, 独立0x01, 逆潮流不可0x02
	props[0xd0] = new uint8_t[0x02]{0x01, 0x01};
	// 発電要請時刻設定0xhhmmhhmm
	props[0xd1] = new uint8_t[0x05]{0x04, 0xff, 0xff, 0xff, 0xff};
	// 発電要請時指定発電状態: 定格最大0x41, 負荷追従0x42
	props[0xd2] = new uint8_t[0x02]{0x01, 0x41};
};
