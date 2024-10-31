#pragma once
#include "../../Core/Common.h"
#include "Filter.h"
#include <vector>

struct Delay
{
	s32 steps;
	f64 feedback;
	s32 tempo;
	std::vector<f64> history;
	s32 offset;

	void Resize(s32 size)
	{
		history.resize(size);
		offset %= history.size();
	}

	f64 Process(f64 sample)
	{
		u32 history_size = u32(60) * SAMPLE_RATE * steps / (4 * tempo);

		if (history.size() != history_size)
			Resize(history_size);

		f64 output = sample + feedback * history[offset];
		history[offset] = output;

		offset = wrap(offset + 1, s32(history.size()));

		return output;
	}
};