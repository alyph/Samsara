
#include "tablet.h"
#include "shader.h"
#include "math/math_types.h"
#include "core/assertion.h"
#include <cstddef>
#include <algorithm>
#include <GL/glew.h>

static constexpr const char* uniform_mvp = "MVP";
static constexpr const char* uniform_dims = "Dims";
static constexpr const char* attribute_vert_pos = "VertexPos";
static constexpr const char* attribute_coord = "Coordinates";
static constexpr const char* attribute_color1 = "Color1";
static constexpr const char* attribute_color2 = "Color2";
static constexpr const char* attribute_page = "Page";
static constexpr const char* attribute_code = "Code";

Id TabletStore::add_tablet(int width, int height, const Shader& shader)
{
	asserts(shader.id(), "tablet shader must be valid.");
	asserts(width > 0 && height > 0, "tablet cannot be empty sized");

	// find or create a shader cache
	const auto shader_id = shader.id();
	size_t shader_cache_idx = -1;
	for (size_t i = 0; i < shader_caches.size(); i++)
	{
		if (shader_caches[i].shader_id == shader_id)
		{
			shader_cache_idx = i;
			break;
		}
	}

	if (shader_cache_idx == -1)
	{
		shader_cache_idx = shader_caches.size();
		auto& cache = shader_caches.emplace_back();
		cache.shader_id = shader_id;
		cache.param_mvp = shader.uniform_loc(uniform_mvp);
		cache.param_dims = shader.uniform_loc(uniform_dims);
	}

	const Id id = tablets.size();
	auto& tablet = tablets.emplace_back();

	// create vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	tablet.shader_cache_idx = shader_cache_idx;
	tablet.cache.vao = vao;
	tablet.cache.width = width;
	tablet.cache.height = height;

	const size_t num_fixed_glyphs = (width * height);
	tablet.cache.max_num_glyphs = num_fixed_glyphs * 2; // allow twice as many glyphs to support layers

	GLuint vbo;

	const auto vert_loc = shader.attribute_loc(attribute_vert_pos);
	if (vert_loc >= 0)
	{
		IVec2 verts[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		// TODO: use glVertexAttribIPointer
		glVertexAttribIPointer(vert_loc, 2, GL_INT, 0, 0);
		glEnableVertexAttribArray(vert_loc);
	}
	else
	{
		printf("Cannot find shader attribute: %s\n", attribute_vert_pos);
	}

	// setup an initial coord buffer
	std::vector<IVec2> coords(num_fixed_glyphs);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const auto idx = x + y * width;
			coords[idx].x = x;
			coords[idx].y = y;
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, tablet.cache.max_num_glyphs * sizeof(IVec2), nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_fixed_glyphs * sizeof(IVec2), coords.data());
	tablet.cache.coord_buffer = vbo;

	const auto coord_loc = shader.attribute_loc(attribute_coord);
	if (coord_loc >= 0)
	{
		glVertexAttribIPointer(coord_loc, 2, GL_INT, 0, 0);
		// TODO: use glVertexAttribIPointer
		glEnableVertexAttribArray(coord_loc);
		glVertexAttribDivisor(coord_loc, 1);
	}
	else
	{
		printf("Cannot find shader attribute: %s\n", attribute_coord);
	}

	// generate the glyph data buffer (this buffer has all the color, 
	// code point data combined into a big buffer and will be updated per frame)
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, tablet.cache.max_num_glyphs * sizeof(GlyphData), nullptr, GL_STREAM_DRAW);
	tablet.cache.glyph_buffer = vbo;

	auto setup_glyph_data_attribute = [&shader](const char* attr, GLint size, GLenum type, GLboolean normalized, const GLvoid* offset)
	{
		const auto attr_loc = shader.attribute_loc(attr);
		if (attr_loc >= 0)
		{
			glVertexAttribPointer(attr_loc, size, type, normalized, sizeof(GlyphData), offset);
			glEnableVertexAttribArray(attr_loc);
			glVertexAttribDivisor(attr_loc, 1);
		}
		else
		{
			printf("Cannot find shader attribute: %s\n", attr);
		}
	};

	setup_glyph_data_attribute(attribute_color1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
	// setup_glyph_data_attribute(attribute_color2, 4, GL_UNSIGNED_BYTE, GL_TRUE, (void*)offsetof(GlyphData, color2));
	
	// TODO: use glVertexAttribIPointer
	// setup_glyph_data_attribute(attribute_page, 1, GL_UNSIGNED_BYTE, GL_FALSE, (void*)offsetof(GlyphData, page));
	// setup_glyph_data_attribute(attribute_code, 1, GL_UNSIGNED_BYTE, GL_FALSE, (void*)offsetof(GlyphData, code));


	uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return id;
}

const TabletCache& TabletStore::tablet_cache(Id tablet_id) const
{
	return tablets[tablet_id].cache;
}

const TabletShaderCache& TabletStore::shader_cache(Id tablet_id) const
{
	const auto shader_idx = tablets[tablet_id].shader_cache_idx;
	return shader_caches[shader_idx];
}

namespace renderer
{
	void draw(const TabletStore& store, const TabletDrawStream& stream)
	{
		// TODO: revert states
		// opengl states
		// depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		for (const auto& item : stream.items)
		{
			const auto& tablet_cache = store.tablet_cache(item.tablet_id);
			const auto& shader_cache = store.shader_cache(item.tablet_id);

			// use shader
			glUseProgram(static_cast<GLuint>(shader_cache.shader_id));
			
			// set mvp
			glUniformMatrix4fv(shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());
			glUniform2i(shader_cache.param_dims, tablet_cache.width, tablet_cache.height);

			glBindVertexArray(static_cast<GLuint>(tablet_cache.vao));

			asserts(tablet_cache.width * tablet_cache.height + item.extra_coords.size() == item.glyphs.size());
			const auto num_glyphs = std::min(static_cast<size_t>(tablet_cache.max_num_glyphs), item.glyphs.size());
			const auto num_fixed_glyphs = (tablet_cache.width * tablet_cache.height);

			// copy in extra coordinates
			if (!item.extra_coords.empty())			
			{
				glBindBuffer(GL_ARRAY_BUFFER, tablet_cache.coord_buffer);
				glBufferSubData(GL_ARRAY_BUFFER, num_fixed_glyphs * sizeof(IVec2), item.extra_coords.size() * sizeof(IVec2), item.extra_coords.data());
			}

			// copy in the glyph data
			glBindBuffer(GL_ARRAY_BUFFER, tablet_cache.glyph_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, item.glyphs.size() * sizeof(GlyphData), item.glyphs.data());

			// Draw all glyphs instanced
			// TODO: another way to draw this is to draw a quad, and have the pixel shader 
			// fill in the content based on uv
			// 6 because of two triangles, see above indices in add_tablet()
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, num_glyphs);
			// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

		}
	}
}