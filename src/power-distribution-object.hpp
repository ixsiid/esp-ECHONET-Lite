#pragma once
#include "ECHONETlite-object.hpp"

class PowerDistribution : public ELObject {
    public:
	static const uint16_t class_u16 = 0x8702;

    private:
	static const char TAG[8];
	uint8_t ch_count;

	typedef struct {
		portTickType tick;
		uint8_t voltage;
		uint16_t ampere;
		uint32_t watt_tick;
	} measure_t;
	measure_t * measures;

    public:
	PowerDistribution(uint8_t instance, uint8_t ch_count);
	void update();
	void update(uint8_t ch);
	void set_ampere(uint8_t ch, uint16_t ampere);
};
