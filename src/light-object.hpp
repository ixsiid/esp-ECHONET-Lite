#pragma once
#include "ECHONETlite-object.hpp"

class Light : public ELObject {
	public:
	static const uint16_t class_u16 = 0x9002;

	private:
	static const char TAG[9];

	public:
	Light(uint8_t instance);
};