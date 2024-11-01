#pragma once
#include "../../Core/Common.h"
#include "Filter.h"
#include <vector>
#include <iostream>

// Convert from musical time to seconds
static f64 bpm_to_sec(s32 beat, s32 beat_per_bar, s32 bpm) { return (beat / beat_per_bar) * (60.0 / bpm); }
static u32 bpm_to_sample(s32 beat, s32 beat_per_bar, s32 bpm, f64 sample_rate) { return 60 * beat * sample_rate / (bpm * beat_per_bar); }

struct Delay
{
	s32 beat;
	f64 feedback;
	s32 bpm;
	s32 beat_per_bar;
	std::vector<f64> history;
	s32 offset;

	void Resize(s32 size)
	{
		history.resize(size);
		offset %= history.size();
	}

	f64 Process(f64 sample)
	{
		u32 history_size = bpm_to_sample(beat, beat_per_bar, bpm, SAMPLE_RATE);
		if (history.size() != history_size)
			Resize(history_size);

		// Compute delay and store in history
		f64 output = sample + feedback * history[offset];
		history[offset] = output;

		// Increment offset
		offset = wrap(offset + 1, u32(history.size()));

		return output;
	}
};