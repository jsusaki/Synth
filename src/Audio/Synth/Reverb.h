#pragma once

#include "Filter.h"


// signalsmith: https://signalsmith-audio.co.uk/writing/2021/lets-write-a-reverb/
// vallhalla DSP: https://valhalladsp.com/2021/09/20/getting-started-with-reverb-design-part-1-dev-environments/

// Freeverb: https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
struct ReverbEffect
{
	// Freeverb constants
	static constexpr f64 MAX_SPREAD          = 100;
	static constexpr s32 NUM_COMB_FILTERS    = 8;
	static constexpr s32 NUM_ALLPASS_FILTERS = 4;
	// Parallel Low-pass Feedback Comb Filters
	CombFilter    comb_filters[NUM_COMB_FILTERS];
	// Serial All-pass Feedback Filters
	AllPassFilter allpass_filters[NUM_ALLPASS_FILTERS];
	// Reverb parameters
	f64 room;
	f64 spread;
	f64 damp;
	f64 decay;
	f64 dry;
	f64 wet;

	// Config
	void ComputeFilterDelays() 
	{
		s32 comb_filter_delays[NUM_COMB_FILTERS] = { 1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116 };
		for (s32 i = 0; i < NUM_COMB_FILTERS; i++)
		{
			comb_filters[i].SetDelay(room * (comb_filter_delays[i] + spread * MAX_SPREAD));
			comb_filters[i].damp = damp;
		}

		// Compute comb feedbacks
		for (s32 i = 0; i < NUM_COMB_FILTERS; i++)
		{
			f64 delay_in_seconds = comb_filters[i].history.size() * 1.0 / SAMPLE_RATE;
			comb_filters[i].feedback = std::pow(10.0, -3.0 * delay_in_seconds / decay);
		}

		// Compute all pass feedbacks
		s32 allpass_delays[NUM_ALLPASS_FILTERS] = { 225, 556, 441, 341 };
		for (s32 i = 0; i < NUM_ALLPASS_FILTERS; i++)
		{
			allpass_filters[i].SetDelay(room * (allpass_delays[i] + spread * MAX_SPREAD));
			allpass_filters[i].feedback = 0.5;
		}
	}

	f64 Process(f64 sample)
	{
		f64 linear_dry = db_to_linear(dry);
		f64 linear_wet = db_to_linear(wet);
		f64 output = 0.0;

		// Apply Comb Filters in parallel
		for (s32 f = 0; f < NUM_COMB_FILTERS; f++)
			output += comb_filters[f].Process(sample);

		// Normalize
		output /= NUM_COMB_FILTERS;

		// Apply All Pass Filters in series
		for (s32 f = 0; f < NUM_ALLPASS_FILTERS; f++)
			output = allpass_filters[f].Process(output);

		// Normalize
		output /= NUM_ALLPASS_FILTERS;

		output = linear_dry * sample + linear_wet * output;

		return output;
	}
};