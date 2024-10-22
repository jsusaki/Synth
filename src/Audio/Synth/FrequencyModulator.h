#pragma once

#include "../../Core/Common.h"

struct FrequencyModulator
{
	f64 amplitude;
	f64 frequency;

	FrequencyModulator(f64 amp = 0.0, f64 freq = 0.0) {}

	f64 Modulate(f64 time_step, f64 carrier_frequency)
	{
		return amplitude * carrier_frequency * std::sin(frequency * 2.0 * PI * time_step);
	}
};