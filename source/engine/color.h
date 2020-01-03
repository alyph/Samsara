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

constexpr inline Color to_color(const Color32 color)
{
	return 
	{ 
		color.r / 255.f,
		color.g / 255.f,
		color.b / 255.f,
		color.a / 255.f,
	};
}

constexpr Color operator"" _rgb(unsigned long long v)
{
	uint8_t r = (v >> 16 & 0xff);
	uint8_t g = (v >> 8 & 0xff);
	uint8_t b = (v & 0xff);
	return to_color(Color32{ r, g, b, 255 });
}
