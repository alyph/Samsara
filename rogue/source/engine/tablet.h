#pragma once

#include "color.h"
#include "id.h"
#include "math_types.h"
#include "presenter.h"
#include "buffer.h"
#include <vector>
#include <memory>

struct GlyphData;

namespace attrs
{
	// extern Attribute<Id> tablet_id; // TODO: remove
	extern Attribute<Id> quad_shader;
	extern Attribute<Buffer<GlyphData>> glyphs;
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

namespace elem
{
	extern Id tablet(const Context ctx);
}