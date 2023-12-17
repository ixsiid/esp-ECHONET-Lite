#pragma once
#include "ECHONETlite-object.hpp"

class PowerDistribution : public ELObject {
    public:
	static const uint16_t class_u16 = 0x8702;

    private:
	static const char TAG[8];
	uint16_t * amperes;
	portTickType * ticks;
	uint8_t ch_count;

    public:
	PowerDistribution(uint8_t instance, uint8_t ch_count);
	void update();
	void update(uint8_t ch);
	void set_ampere(uint8_t ch, uint16_t ampere);
};
