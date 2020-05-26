#pragma once

#include <cstdint>
#include "engine/array.h"
#include "engine/color.h"
#include "engine/string.h"
#include "engine/math_types.h"

using TypeIndex = uint16_t;

struct TerrainType
{
	String name;
	uint16_t glyph;
	Color32 color_a;
	Color32 color_b;
	uint32_t flags{};
};

struct CityRules
{
	TypeIndex starting_dev_type;
	TypeIndex starting_wall_type;
};

enum class StructureCategory: uint8_t
{
	none,
	dev,
	wall,
	road,
};

struct StructureType
{
	String name;
	uint16_t glyph;
	Color32 color;
	StructureCategory category;
	TypeIndex entity_type;
};

enum class DevelopmentArea: uint8_t
{
	urban,
	rural,
};

struct DevelopmentType
{
	String name;
	DevelopmentArea area;
	TypeIndex structure_type;
};

struct Globals
{
	CityRules city_rules;
	Array<TerrainType> terrain_types;
	Array<StructureType> structure_types;
	Array<DevelopmentType> development_types;
};

