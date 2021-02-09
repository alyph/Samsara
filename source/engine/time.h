#pragma once
#include <cstdint>


struct Time
{
	int64_t current_time_ns{};
	int64_t delta_time_ns{};

	inline double current_time_seconds() const { return current_time_ns / 1.0e9; }
	inline double delta_time_seconds() const { return delta_time_ns / 1.0e9; }
};


