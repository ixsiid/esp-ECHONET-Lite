#pragma once
#include "ECHONETlite-object.hpp"

class InstantHeater : public ELObject {
	public:
	static const uint16_t class_u16 = 0x7202;

	private:
	static const char TAG[18];

	public:
	InstantHeater(uint8_t instance);
};