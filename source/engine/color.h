#pragma once

#include <cstdint>
#include <cmath>

class Color
{
public:
	float r{}, g{}, b{}, a{};
};

class Color32
{
public:
	uint8_t r{}, g{}, b{}, a{};
};

inline Color32 to_color32(const Color& color)
{
	return 
	{ 
		(uint8_t)std::round(255 * color.r),
		(uint8_t)std::round(255 * color.g),
		(uint8_t)std::round(255 * color.b),
		(uint8_t)std::round(255 * color.a),
	};
}

