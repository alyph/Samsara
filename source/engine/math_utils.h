#pragma once
#include "math_consts.h"
#include <cmath>

inline float to_rad(float deg)
{
	static const constexpr float to_rad_s = (consts::pi / 180.f);
	return deg * to_rad_s;
}

inline float to_deg(float rad)
{
	static const constexpr float to_deg_s = (180.f / consts::pi);
	return rad * to_deg_s;
}

inline bool equal(float a, float b, float eps)
{
	return std::abs(a - b) <= eps;
}

