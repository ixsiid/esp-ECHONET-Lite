#pragma once
#include "ECHONETlite-object.hpp"

class EVPS : public ELObject {
    public:
	static const uint16_t class_u16 = 0x7e02;

	enum class Mode : uint8_t {
		Charge			 = 0x42,
		Discharge			 = 0x43,
		Stanby			 = 0x44,
		Charge_And_Discharge = 0x46,
		Stop				 = 0x47,
		Starting			 = 0x48,
		Auto				 = 0x49,
		Undefine			 = 0x40,
		Unacceptable		 = 0x00,
	};

    private:
	enum class Type : uint8_t {
		AC_CPLT = 0x11,
		AC_HLC_ONLY_CHARGING = 0x12,
		AC_HLC = 0x13,

		DC_AA_ONLY_CHARGING    = 0x21,
		DC_AA                  = 0x22,
		DC_AA_ONLY_DISCHARGING = 0x23,

		DC_BB_ONLY_CHARGING    = 0x31,
		DC_BB                  = 0x32,
		DC_BB_ONLY_DISCHARGING = 0x33,

		DC_EE_ONLY_CHARGING    = 0x41,
		DC_EE                  = 0x42,
		DC_EE_ONLY_DISCHARGING = 0x43,

		DC_FF_ONLY_CHARGING    = 0x51,
		DC_FF                  = 0x52,
		DC_FF_ONLY_DISCHARGING = 0x53,
	};

	static const char TAG[8];

	uint64_t accumulation_charging_watt_tick;
	uint64_t accumulation_discharging_watt_tick;
	portTickType timer;

    public:
	EVPS(uint8_t instance);

	void set_mode(Mode mode);
	void update();
	void update_input_output(int watt);
	void update_remain_battery_ratio(uint8_t ratio); // 0 - 100%, 0x00 - 0x64
};
