#pragma once

#include "assertion.h"
#include <cmath>

enum class ScalarFlag
{
	undefined,
	absolute,
	relative,
	proportional,
};


struct Scalar
{
	double value{};
	ScalarFlag flag{};

	constexpr inline Scalar() = default;
	constexpr inline Scalar(double v, ScalarFlag f): value(v), flag(f) {}
	constexpr inline Scalar(double v): value(v), flag(ScalarFlag::absolute) {}
	constexpr inline Scalar(int v): value(static_cast<double>(v)), flag(ScalarFlag::absolute) {}
	constexpr inline Scalar(long long v): value(static_cast<double>(v)), flag(ScalarFlag::absolute) {}
	constexpr inline Scalar operator-() const { return { -value, flag }; }
	constexpr inline bool undefined() const { return flag == ScalarFlag::undefined; }
	inline int to_int() const;
	inline long long to_long() const;
	inline double to_double() const;
	inline float to_float() const { return static_cast<float>(to_double()); }
};

static const constexpr Scalar undefined_scalar{};

constexpr Scalar operator"" _rel(long double v)
{
	return { static_cast<double>(v), ScalarFlag::relative };
}

constexpr Scalar operator"" _rel(unsigned long long v)
{
	return { static_cast<double>(v), ScalarFlag::relative };
}

constexpr Scalar operator"" _percent(long double v)
{
	return { static_cast<double>(v) / 100.0, ScalarFlag::proportional };
}

constexpr Scalar operator"" _percent(unsigned long long v)
{
	return { static_cast<double>(v) / 100.0, ScalarFlag::proportional };
}

inline int Scalar::to_int() const
{ 
	asserts(flag == ScalarFlag::absolute);
	return static_cast<int>(std::lround(value));
}

inline long long Scalar::to_long() const
{ 
	asserts(flag == ScalarFlag::absolute); 
	return std::llround(value);
}

inline double Scalar::to_double() const
{ 
	asserts(flag == ScalarFlag::absolute); 
	return value;
}
