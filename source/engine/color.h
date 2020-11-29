#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

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
		(uint8_t)std::clamp((int)std::round(255 * color.r), 0, 255),
		(uint8_t)std::clamp((int)std::round(255 * color.g), 0, 255),
		(uint8_t)std::clamp((int)std::round(255 * color.b), 0, 255),
		(uint8_t)std::clamp((int)std::round(255 * color.a), 0, 255),
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

constexpr Color32 operator"" _rgb32(unsigned long long v)
{
	uint8_t r = (v >> 16 & 0xff);
	uint8_t g = (v >> 8 & 0xff);
	uint8_t b = (v & 0xff);
	return Color32{ r, g, b, 255 };
}

constexpr Color32 operator"" _rgba32(unsigned long long v)
{
	uint8_t r = (v >> 24 & 0xff);
	uint8_t g = (v >> 16 & 0xff);
	uint8_t b = (v >> 8 & 0xff);
	uint8_t a = (v & 0xff);
	return Color32{ r, g, b, a };
}

constexpr Color operator"" _rgb(unsigned long long v)
{
	uint8_t r = (v >> 16 & 0xff);
	uint8_t g = (v >> 8 & 0xff);
	uint8_t b = (v & 0xff);
	return to_color(Color32{ r, g, b, 255 });
}

constexpr Color operator"" _rgba(unsigned long long v)
{
	uint8_t r = (v >> 24 & 0xff);
	uint8_t g = (v >> 16 & 0xff);
	uint8_t b = (v >> 8 & 0xff);
	uint8_t a = (v & 0xff);
	return to_color(Color32{ r, g, b, a });
}

inline Color operator*(const Color& c, float s)
{
	return
	{
		c.r * s,
		c.g * s,
		c.b * s,
		c.a * s,
	};
}

inline Color32 operator*(const Color32& c, float s)
{
	return to_color32(to_color(c) * s);
}

inline Color32 power(const Color32& c, float p)
{
	const auto c2 = to_color(c);
	return to_color32(Color
	{
		std::pow(c2.r, p),
		std::pow(c2.g, p),
		std::pow(c2.b, p),
		std::pow(c2.a, p),
	});
}

