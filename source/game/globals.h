#pragma once

#include <cstdint>
#include "engine/array.h"
#include "engine/color.h"
#include "engine/string.h"
#include "engine/math_types.h"


struct TerrainType
{
	String name;
	uint16_t glyph;
	Color32 color_a;
	Color32 color_b;
	uint32_t flags{};
};

struct Globals
{
	Array<TerrainType> terrain_types;
};

