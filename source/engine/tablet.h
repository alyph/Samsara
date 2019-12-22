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

namespace elem
{
	extern Id tablet(const Context ctx);
}

extern Vec2 calc_tablet_size(int width, int height, Id texture);

struct GlyphData
{
	IVec2 coords;
	IVec2 size{1,1};
	Color32 color1;
	Color32 color2;
	uint8_t code{};
	uint8_t page{};
	uint8_t flags{};
	uint8_t reserved{};
};

struct TabletRenderBuffer
{
	struct Block
	{
		Id elem_id;
		size_t buffer_idx;
	};

	SimpleArray<GlyphData> glyphs;
	SimpleArray<Block> blocks;
	Id last_pushed_elem_id{};

	inline size_t push_glyph(Id elem_id, const GlyphData& glyph); // TODO: optimization: maybe allow emplace to avoid copy
	inline GlyphData& mutate_glyph(size_t idx);
	inline void pop_glyph(size_t idx);
};


// TabletRenderBuffer

inline size_t TabletRenderBuffer::push_glyph(Id elem_id, const GlyphData& glyph)
{
	if (elem_id != last_pushed_elem_id) 
	{ 
		// push_block(elem_id);
		blocks.push_back({ elem_id, glyphs.size() });
		last_pushed_elem_id = elem_id;
	}
	glyphs.push_back(glyph);
	return (glyphs.size() - 1);
}

inline GlyphData& TabletRenderBuffer::mutate_glyph(size_t idx)
{
	// making sure the glyph is at original location 
	// e.g. rearrangement hasn't happened yet
	// asserts(idx < glyphs.size() && idx >= first_intact_buffer_idx);
	return glyphs[idx];
}

inline void TabletRenderBuffer::pop_glyph(size_t idx)
{
	// NOTE: only allow to pop the last one for now
	// asserts(idx == (glyphs.size() - 1) && idx >= first_intact_buffer_idx);
	asserts(idx == (glyphs.size() - 1));
	asserts(blocks.back().buffer_idx <= idx);
	glyphs.pop_back();
}
