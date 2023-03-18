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
	static const char TAG[8];

    public:
	EVPS(uint8_t instance);

	void set_mode(Mode mode);
	void set_input_output(int watt);
};
