#pragma once

#include "color.h"
#include "id.h"
#include "math_types.h"
#include "presenter.h"
#include "array.h"
#include <vector>
#include <memory>

struct GlyphData;

struct TabletLayout
{
	int left{}, top{}, width{}, height{};
};

namespace attrs
{
	// extern Attribute<Id> tablet_id; // TODO: remove
	extern Attribute<Id> quad_shader;
	extern Attribute<SimpleArray<GlyphData>> glyphs;
	extern Attribute<TabletLayout> tablet_layout;
}

struct GlyphData
{
	Color32 color1;
	Color32 color2;
	uint8_t page;
	uint8_t code;
	uint8_t flags;
	uint8_t reserved;
};

// TODO: remove
// extern Id add_tablet(int width, int height, Id texture, Id shader, Id screen_shader);

extern Vec2 calc_tablet_size(int width, int height, Id texture);

namespace elem
{
	extern Id tablet(const Context ctx);
}