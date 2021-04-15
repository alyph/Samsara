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

struct StartingCard
{
	Id type{};
	uint16_t count;
};

struct Defines
{
	Id starting_wall_type{};
	Id urban_vacancy_type{};
	Array<StartingCard> starting_deck{};
};

enum class StructureFlags: uint32_t
{
	none		= 0x00000000,
	dev 		= 0x00000001,
	wall 		= 0x00000002,
	road 		= 0x00000004,
	urban 		= 0x00000010,
	rural 		= 0x00000020,
	vacancy 	= 0x00000040,

};

struct StructureType
{
	Id id{};
	String name;
	uint16_t glyph;
	Color32 color;
	StructureFlags flags;
	Id entity_type;
};

enum class DevelopmentArea: uint8_t
{
	urban,
	rural,
	count,
};

struct DevelopmentType
{
	String name;
	DevelopmentArea area;
	Id structure_type;
};

struct CardType
{
	Id id;
	String name;
	String phrase;
	unsigned int weight{};
};

struct Globals
{
	Defines defines;
	Collection<TerrainType> terrain_types;
	Collection<StructureType> structure_types;
	DevelopmentType development_types[(int)DevelopmentArea::count]{};

	Collection<CardType> card_types;
};

