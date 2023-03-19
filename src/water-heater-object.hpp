#pragma once
#include "ECHONETlite-object.hpp"

class WaterHeater : public ELObject {
    public:
	static const uint16_t class_u16 = 0x6b02;

    private:
	static const char TAG[8];

    public:
	WaterHeater(uint8_t instance);
};
