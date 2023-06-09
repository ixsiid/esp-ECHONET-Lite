#pragma once
#include "udp-socket.hpp"

namespace ELConstant {
constexpr char EL_MULTICAST_IP[] = "224.0.23.0";
const size_t EL_BUFFER_SIZE	   = 256;
const uint16_t EL_PORT		   = 3610;

const size_t _MAX_INSTANCE = 10;
};  // namespace ELConstant

// to HEMS用
class ELObject {
    public:
#pragma pack(1)
	typedef struct {
		uint16_t _1081;
		uint16_t packet_id;
		uint16_t src_device_class;
		uint8_t src_device_id;
		uint16_t dst_device_class;
		uint8_t dst_device_id;
		uint8_t esv;
		uint8_t epc_count;
	} elpacket_t;
#pragma pack()

	enum class SetRequestResult {
		Reject,
		Accept,
		Customize,
	};
	typedef SetRequestResult (*set_cb_t)(ELObject * self, uint8_t epc, uint8_t length, uint8_t* current_buffer, uint8_t* request_buffer);
	typedef void (*get_cb_t)(ELObject* self, uint8_t epc, uint8_t length, uint8_t* current_buffer);

    protected:
    private:
	static const char TAG[8];

    protected:
	static uint8_t buffer[ELConstant::EL_BUFFER_SIZE + 32];
	static size_t buffer_length;
	static elpacket_t* p;
	static uint8_t* epc_start;

	static uint8_t* maker_code;

	static const uint16_t CLASS_HEMS = 0xff05;

	static uint8_t* generate_identify(ELObject* object);

	uint8_t* props[0xff];
	uint8_t get(uint8_t* epcs, uint8_t epc_count);
	uint8_t set(uint8_t* epcs, uint8_t epc_count);

    public:
	const uint8_t instance;
	const uint16_t class_group;
	const uint8_t class_id;
	const uint8_t group_id;

	set_cb_t set_cb;
	get_cb_t get_cb;

	ELObject(uint8_t instance, uint16_t class_group);

	int send(UDPSocket* udp, const esp_ip_addr_t* addr);
	bool process(const elpacket_t* recv, uint8_t* epc_array);
	void notify(uint8_t epc);
};

class Profile : public ELObject {
    public:
	static const uint16_t class_u16 = 0xf00e;

	typedef union {
		uint8_t buffer[ELConstant::EL_BUFFER_SIZE];
		struct {
			elpacket_t p;
			uint8_t epcs[ELConstant::EL_BUFFER_SIZE - sizeof(elpacket_t)];
		};
	} el_packet_buffer_t;

    private:
	static const char TAG[8];

	uint8_t set(uint8_t* epcs, uint8_t epc_count);

	typedef struct {
		uint16_t class_group;
		ELObject * instance;
	} object_t;
	object_t instances[ELConstant::_MAX_INSTANCE];
	int instance_count;

    public:
	Profile(uint8_t major_version, uint8_t minor_version);

	Profile* add(ELObject* object);

	bool process_all_instance(UDPSocket * udp, el_packet_buffer_t * buffer);

	/*
	Profile operator<<(ELObject& object) {
		return this->add(object);
	};
	*/
};
