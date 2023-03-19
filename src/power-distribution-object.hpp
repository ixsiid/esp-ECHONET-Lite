#pragma once
#include "ECHONETlite-object.hpp"

class PowerDistribution : public ELObject {
    public:
	static const uint16_t class_u16 = 0x8702;

    private:
	static const char TAG[8];

    public:
	PowerDistribution(uint8_t instance);
};
