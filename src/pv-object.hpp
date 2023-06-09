#pragma once
#include "ECHONETlite-object.hpp"

class PV : public ELObject {
    public:
	static const uint16_t class_u16 = 0x7902;

    private:
	static const char TAG[8];

	portTickType timer;
	uint64_t accumulation_watt_tick;

    public:
	PV(uint8_t instance);
	void update();
	void update(uint16_t watt);
};
