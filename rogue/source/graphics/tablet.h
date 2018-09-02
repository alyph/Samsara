#pragma once

#include "color.h"
#include "types/id.h"
#include "math/math_types.h"
#include <vector>
#include <memory>

class Shader;

struct GlyphData
{
	Color32 color1;
	Color32 color2;
	uint8_t page;
	uint8_t code;
	uint8_t flags;
	uint8_t reserved;
};

struct TabletCache
{
	Id vao;
	Id coord_buffer;
	Id glyph_buffer;
	Id texture;
	int width;
	int height;
	int max_num_glyphs;
};

struct TabletShaderCache
{
	Id shader_id;
	int param_mvp;
	int param_dims;
};

class TabletStore
{
public:
	Id add_tablet(int width, int height, Id texture, const Shader& shader);
	const TabletCache& tablet_cache(Id tablet_id) const;
	const TabletShaderCache& shader_cache(Id tablet_id) const;

private:
	struct InternalTabletObject
	{
		TabletCache cache;
		Id vert_buffer;
		size_t shader_cache_idx{};		
	};

	std::vector<InternalTabletObject> tablets;
	std::vector<TabletShaderCache> shader_caches;
};

struct TabletDrawItem
{
	Id tablet_id;
	Mat44 mvp;
	std::vector<IVec2> extra_coords;
	std::vector<GlyphData> glyphs;
};

class TabletDrawStream
{
public:
	std::vector<TabletDrawItem> items;
};

namespace renderer
{
	void draw(const TabletStore& store, const TabletDrawStream& stream);
}
