#include <esp_log.h>
#include <esp_netif_ip_addr.h>
#include "ECHONETlite-object.hpp"

/// ELObject

const char ELObject::TAG[] = "EL  Obj";

uint8_t ELObject::buffer[]	    = {};
size_t ELObject::buffer_length    = sizeof(elpacket_t);
ELObject::elpacket_t* ELObject::p = (elpacket_t*)ELObject::buffer;
uint8_t* ELObject::epc_start	    = buffer + sizeof(elpacket_t);

uint8_t* ELObject::maker_code = new uint8_t[4]{0x03, 0xff, 0xff, 0xff};  // 開発用

uint8_t* ELObject::generate_identify(ELObject * object) {
	uint8_t* buffer = new uint8_t[0x12]{0x11, 0xfe,
								 maker_code[1], maker_code[2], maker_code[3], 0x00,
								 'i', 'x', 's', 'i', 'i', 'd', 0x00, 0x00, 0x00,
								 object->group_id, object->class_id, object->instance};
	return buffer;
}

ELObject::ELObject(uint8_t instance, uint16_t class_group)
 : instance(instance), class_group(class_group)
 , class_id(class_group >> 8), group_id(class_group & 0xff)
{
	p->_1081			= 0x8110;
	p->dst_device_class = ELObject::CLASS_HEMS;
	p->dst_device_id	= 0x01;

	memset(props, 0, 0xff * sizeof(uint8_t *));
	set_cb = nullptr;
	get_cb = nullptr;
}

int ELObject::send(UDPSocket* udp, const esp_ip_addr_t* addr) {
	int len = udp->write(addr, ELConstant::EL_PORT, buffer, buffer_length);
	ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, len, ESP_LOG_INFO);
	return len;
}

bool ELObject::process(const elpacket_t* recv, uint8_t* epcs) {
	// ESP_LOGI(TAG, "Profile: process");
	// GET
	p->packet_id = recv->packet_id;
	if (recv->esv == 0x62 || recv->esv == 0x61 || recv->esv == 0x60) {
		buffer_length = 0;
		recv->esv == 0x62 ? p->epc_count = get(epcs, recv->epc_count) : p->epc_count = set(epcs, recv->epc_count);

		if (recv->esv == 0x60) return false;

		p->src_device_class = class_group;
		p->src_device_id	= instance;

		if (p->epc_count > 0) {
			p->esv = recv->esv + 0x10;
		} else {
			p->esv					 = recv->esv - 0x10;
			buffer[sizeof(elpacket_t) + 0] = 0;
			buffer_length				 = sizeof(elpacket_t) + 1;
		}

		// ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, buffer_length, ESP_LOG_INFO);
		return true;
	}

	return false;
};

uint8_t ELObject::get(uint8_t* epcs, uint8_t count) {
	ESP_LOGI(TAG, "(%hx) Get request %d", class_group, count);
	p->src_device_class = class_group;
	p->src_device_id	= instance;

	uint8_t* t = epcs;
	uint8_t* n = epc_start;
	uint8_t res_count = 0;

	for (int i = 0; i < count; i++) {
		uint8_t epc = t[0];
		uint8_t len = t[1];
 		ESP_LOGD(TAG, "(%hx) EPC 0x%02x [%d]", class_group, epc, len);
		t += 2;

		if (len > 0) ESP_LOG_BUFFER_HEXDUMP(TAG, t, len, ESP_LOG_INFO);
		t += len;

		if (props[epc] == nullptr) continue;
		if (get_cb) get_cb(this, epc, props[epc][0], &(props[epc][1]));

		*n = epc;
		n++;
		memcpy(n, props[epc], props[epc][0] + 1);
		n += props[epc][0] + 1;

		res_count++;
	}

	buffer_length = sizeof(elpacket_t) + (n - epc_start);

	return res_count;
};

uint8_t ELObject::set(uint8_t* epcs, uint8_t count) {
	ESP_LOGI(TAG, "(%hx) Set request %d", class_group, count);
	p->src_device_class = class_group;
	p->src_device_id	= instance;

	uint8_t* t = epcs;
	uint8_t* n = epc_start;
	uint8_t res_count = 0;

	for (int i = 0; i < count; i++) {
		uint8_t epc = t[0];
		uint8_t len = t[1];
		ESP_LOGI(TAG, "(%hx) EPC 0x%02x [%d]", class_group, epc, len);
		t += 2;

		if (props[epc] == nullptr) continue;

		if (set_cb == nullptr) {
			t += len;
			continue;
		}
		
		if (set_cb(this, epc, props[epc][0], &(props[epc][1]), t) == SetRequestResult::Reject) {
			t += len;
			continue;
		}

		// メモリ更新後の値を返却する
		*n = epc;
		n++;
		memcpy(n, props[epc], props[epc][0] + 1);
		n += props[epc][0] + 1;

		res_count++;
	}

	buffer_length = sizeof(elpacket_t) + (n - epc_start);

	return res_count;
};

void ELObject::notify(uint8_t epc) {
	p->epc_count  = 1;
	p->esv	    = 0x74;  // ESV_INFC
	epc_start[0]  = epc;
	epc_start[1]  = props[epc][0];
	epc_start[2]  = props[epc][1];
	buffer_length = sizeof(elpacket_t) + 3;
};


//// Profile
const char Profile::TAG[] = "EL Prof";

// Versionは、ECHONET lite規格書のVersion
// not 機器オブジェクト詳細規定
Profile::Profile(uint8_t major_version, uint8_t minor_version) : ELObject(1, Profile::class_u16) {
	props[0x8a] = maker_code;
	props[0x82] = new uint8_t[0x05]{0x04, major_version, minor_version, 0x01, 0x00};
	props[0x83] = generate_identify(this);
	props[0xd6] = new uint8_t[0x20]{0x01, 0x00};
}

Profile* Profile::add(ELObject* object) {
	int i = props[0xd6][1];
	if (i >= 10) {
		ESP_LOGE(TAG, "Possible to regist object less than 11");
		return this;
	}
	props[0xd6][2 + i * 3 + 0] = object->group_id;
	props[0xd6][2 + i * 3 + 1] = object->class_id;
	props[0xd6][2 + i * 3 + 2] = object->instance;

	props[0xd6][1] += 1;
	props[0xd6][0] += 3;

	return this;
};
