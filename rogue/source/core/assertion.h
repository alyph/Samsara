#pragma once

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

static inline void assertion_fail(const char* expr, const char* file, int line, const char* func)
{
	std::fprintf(stderr, "Assertion failed: '%s' in %s (%s:%d)\n", expr, func, file, line);
	std::abort();
}

static inline void assertion_fail(const char* expr, const char* file, int line, const char* func, const char* message, ...)
{
	std::fprintf(stderr, "Assertion failed: '%s' in %s (%s:%d)\n>>> ", expr, func, file, line);

	va_list args;
	va_start(args, message);
	std::vfprintf(stderr, message, args);
	va_end(args);

	std::fprintf(stderr, "\n");
	std::abort();
}

#define asserts(expr, ...) { if (!(expr)) assertion_fail(#expr, __FILE__, __LINE__, __func__, __VA_ARGS__);  }