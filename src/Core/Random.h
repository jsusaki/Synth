#pragma once
#include <random>
#include <chrono>
#include <optional>

#include "Common.h"

template <typename F, typename I>
class random
{
public:
	random(u32 seed = std::random_device{}()) : mt(seed) {}

public:
	void seed(std::optional<u32> seed = std::nullopt)
	{
		if (seed.has_value()) 
		{
			mt.seed(seed.value());
		}
		else 
		{
			auto now = std::chrono::high_resolution_clock::now();
			mt.seed(std::random_device{}() ^ now.time_since_epoch().count());
		}
	}

	inline F uniform(F min, F max)
	{
		std::uniform_real_distribution<F>dist(min, max);
		return dist(mt);
	}

	inline I uniformi(I min, I max)
	{
		std::uniform_int_distribution<I>dist(min, max);
		return dist(mt);
	}

	inline F normal(F mu, F sigma)
	{
		std::normal_distribution<F>dist(mu, sigma);
		return dist(mt);
	};

	inline bool bernoulli(F p) 
	{
		std::bernoulli_distribution dist(p);
		return dist(mt);
	}

	inline std::vector<F> uniform(F min, F max, I size)
	{
		std::vector<F> res(size);
		for (I i = 0; i < res.size(); i++)
			res[i] = uniform(min, max);
		return res;
	}

	inline std::vector<I> uniformi(I min, I max, I size)
	{
		std::vector<I> res(size);
		for (I i = 0; i < res.size(); i++)
			res[i] = uniformi(min, max);
		return res;
	}

	inline std::vector<F> normal(F mu, F sigma, I size)
	{
		std::vector<F> res(size);
		for (I i = 0; i < res.size(); i++)
			res[i] = normal(mu, sigma);
		return res;
	}

	inline std::vector<F> bernoulli(F p, I size)
	{
		std::vector<F> res(size);
		for (I i = 0; i < res.size(); i++)
			res[i] = bernoulli(p);
		return res;
	}

private:
	std::mt19937 mt;
};

using randf32 = random<f32, s32>;
using randf64 = random<f64, s32>;