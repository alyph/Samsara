#pragma once


#include <random>
#include <cstdint>

extern std::default_random_engine global_rand_engine;

// generate random integer between [a, b] (inclusive)
inline int rand_int(int a, int b)
{
	return std::uniform_int_distribution<>(a, b)(global_rand_engine);
}

// generate random integer between [0, n-1] (inclusive)
inline int rand_int(int n)
{
	return std::uniform_int_distribution<>(0, n-1)(global_rand_engine);
}

inline uint64_t rand_int(uint64_t n)
{
	return std::uniform_int_distribution<uint64_t>(0, n-1)(global_rand_engine);
}

