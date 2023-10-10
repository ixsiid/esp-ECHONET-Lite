#pragma once
#include "ECHONETlite-object.hpp"

class FuelCell : public ELObject {
	public:
	static const uint16_t class_u16 = 0x7c02;

	private:
	static const char TAG[13];

	public:
	FuelCell(uint8_t instance);
};