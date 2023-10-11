const fs = require('fs');
const assert = require('node:assert/strict');

process.argv.filter((_, i) => i >= 2)
	.forEach(file => {
		console.log(`Start generate from '${process.argv[2]}'`);

		const data = require(file);

		// フォーマットエラー
		console.log('data format check...')
		assert(typeof (data.name) === 'string');

		assert(typeof (data.group) === 'string');
		assert(data.group.startsWith('0x'));

		assert(typeof (data.class) === 'string');
		assert(data.class.startsWith('0x'));

		assert(typeof (data.place) === 'string');
		assert(data.place.startsWith('0b'));

		const props = Object.entries(data.properties).sort(([a], [b]) => parseInt(a, 16) - parseInt(b, 16));
		props.forEach(([k, v]) => {
			assert(k.startsWith('0x'));
			assert(typeof (v.description) === 'string');
			assert(typeof (v.value) === 'string');
			assert(v.value.startsWith('0x'));
		});
		console.log('Done');

		const class_name = data.name.replace(/\s/g, '');
		const file_name = data.name.toLowerCase().replace(/\s/g, '-') + '-object';
		assert(!fs.existsSync(`../src/${file_name}.hpp`));
		assert(!fs.existsSync(`../src/${file_name}.cpp`));

		const gets = [0x81, 0x82, 0x88, 0x8a, 0x9d, 0x9e, 0x9f];
		const sets = [];
		const notify = [];

		const super_props = [
			['0x81', '設置場所', [data.place]],
			['0x82', '規格Version情報: Release Q', ['0x00', '0x00', "'Q'", '0x01']],
			['0x88', '異常発生状態: 異常なし0x42', ['0x42']],
			['0x8a', 'メーカコード', 'maker_code'],
			['0x9d', '状態変更通知アナウンスマップ'],
			['0x9e', 'Setプロパティマップ'],
			['0x9f', 'Getプロパティマップ'],
		];

		notify.push(...props.filter(([_, v]) => v.notify).map(([k, _]) => parseInt(k, 16)));
		sets.push(...props.filter(([_, v]) => v.set).map(([k, _]) => parseInt(k, 16)));
		gets.push(...props.filter(([_, v]) => v.get).map(([k, _]) => parseInt(k, 16)));
		console.log(gets.map(x => '0x' + x.toString(16)));
		const to_map = props => {
			if (props.length < 16) {
				return [props.length, ...props].map(x => '0x' + x.toString(16));
			}

			return ['0x' + props.length.toString(16).padStart(2, '0'),
			'\n//	                                  fedcba98',
			...Array.from(Array(16)).map((_, x) => {
				const t = props.filter(p => (p % 16) == x).map(p => (p - x) / 16);
				return '\n	                                0b'
					+ Array.from(Array(8)).map((_, i) => t.includes(15 - i) ? '1' : '0').join('')
					+ ' /* 0x_' + x.toString(16) + ' */';
			})];
		};
		super_props.find(([k]) => k == '0x9d').push(to_map(notify));
		super_props.find(([k]) => k == '0x9e').push(to_map(sets));
		super_props.find(([k]) => k == '0x9f').push(to_map(gets));

		const hpp = `#pragma once
#include "ECHONETlite-object.hpp"

class ${class_name} : public ELObject {
	public:
	static const uint16_t class_u16 = 0x${data.class.substring(2)}${data.group.substring(2)};

	private:
	static const char TAG[${data.name.length + 1 + 3}];

	public:
	${class_name}(uint8_t instance);
};`;

		super_props.forEach((value) => {
			const [_0, _1, property_value] = value;

			if (typeof (property_value) === 'string') {
				// そのまま利用、現状メーカコードのみ
				value.push(property_value);
				return;
			}

			assert(property_value instanceof Array);
			// データがあるときは1byte目にデータ長を入れる
			const length = property_value.filter(x => !x.startsWith('\n//')).length;
			property_value.unshift('0x' + length.toString(16).padStart(2, '0'));
			value.push(`new uint8_t[0x${(length + 1).toString(16).padStart(2, '0')
				}]{${property_value.join(', ')}}`);
		});

		const cpp = `#include <esp_log.h>
#include "${file_name}.hpp"

const char ${class_name}::TAG[] = "EL ${data.name}";

${class_name}::${class_name}(uint8_t instance) : ELObject(instance, ${class_name}::class_u16) {
	//// スーパークラス
${super_props.map(([k, comment, _, v]) => `	// ${comment}
	props[${k}] = ${v};`).join('\n')}

	//// 固有クラス
${props.map(([k, { description, value }]) => {
			const v = value.substring(2)
				.match(/.{2}/g)
				.map(hex => '0x' + hex);
			if (v.unshift('0x' + v.length.toString(16).padStart(2, '0')));
			return `	// ${description}
	props[${k}] = new uint8_t[0x${v.length.toString(16).padStart(2, '0')}]{${v.join(', ')}};`;
		}).join('\n')}
};
`;

		fs.writeFileSync(`../src/${file_name}.hpp`, hpp);
		fs.writeFileSync(`../src/${file_name}.cpp`, cpp);
		console.log(`Generated: ${file_name}`);
	});
