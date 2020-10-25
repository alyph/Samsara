#pragma once

#include <cstdint>
#include "engine/collection.h"
#include "engine/color.h"
#include "engine/string.h"
#include "engine/math_types.h"

using TypeId = uint16_t;

static inline constexpr TypeId to_type_id(Id id)
{
	return static_cast<TypeId>(id);
}

struct TerrainType
{
	Id id{};
	String name;
	char symbol;
	uint16_t glyph;
	Color32 color_a;
	Color32 color_b;
	uint32_t flags{};
};

struct Defines
{
	Id starting_dev_type;
	Id starting_wall_type;
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
	Id id{};
	String name;
	uint16_t glyph;
	Color32 color;
	StructureCategory category;
	Id entity_type;
};

enum class DevelopmentArea: uint8_t
{
	urban,
	rural,
};

struct DevelopmentType
{
	Id id{};
	String name;
	DevelopmentArea area;
	Id structure_type;
};

struct Globals
{
	Defines defines;
	Collection<TerrainType> terrain_types;
	Collection<StructureType> structure_types;
	Collection<DevelopmentType> development_types;
};

