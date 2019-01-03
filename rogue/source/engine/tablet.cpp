
#include "tablet.h"
#include "shader.h"
#include "math_types.h"
#include "assertion.h"
#include <cstddef>
#include <algorithm>
#include <GL/glew.h>

static constexpr const char* uniform_mvp = "MVP";
static constexpr const char* uniform_dims = "Dims";
static constexpr const char* uniform_atlas = "Atlas";
static constexpr const char* uniform_texture = "Texture";
static constexpr const char* attribute_vert_pos = "VertexPos";
static constexpr const char* attribute_coord = "Coordinates";
static constexpr const char* attribute_color1 = "Color1";
static constexpr const char* attribute_color2 = "Color2";
static constexpr const char* attribute_page = "Page";
static constexpr const char* attribute_code = "Code";
static constexpr const char* attribute_uv = "UV";

Id TabletStore::add_tablet(int width, int height, Id texture, const Shader& shader, const Shader& screen_shader)
{
	asserts(shader.id(), "tablet shader must be valid.");
	asserts(width > 0 && height > 0, "tablet cannot be empty sized");
	asserts(texture != null_id, "tablet requires valid texture to render");

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
		//cache.param_mvp = shader.uniform_loc(uniform_mvp);
		cache.param_dims = shader.uniform_loc(uniform_dims);
	}

	const auto screen_shader_id = screen_shader.id();
	size_t screen_shader_cache_idx = -1;
	for (size_t i = 0; i < screen_shader_caches.size(); i++)
	{
		if (screen_shader_caches[i].shader_id == screen_shader_id)
		{
			screen_shader_cache_idx = i;
			break;
		}
	}

	if (screen_shader_cache_idx == -1)
	{
		screen_shader_cache_idx = screen_shader_caches.size();
		auto& cache = screen_shader_caches.emplace_back();
		cache.shader_id = screen_shader_id;
		cache.param_mvp = screen_shader.uniform_loc(uniform_mvp);
		cache.param_texture = screen_shader.uniform_loc(uniform_texture);
		cache.param_vert = screen_shader.attribute_loc(attribute_vert_pos);
		cache.param_uv = screen_shader.attribute_loc(attribute_uv);
	}


	const Id id = tablets.size();
	auto& tablet = tablets.emplace_back();

	// create vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	tablet.shader_cache_idx = shader_cache_idx;
	tablet.screen_shader_cache_idx = screen_shader_cache_idx;
	tablet.cache.vao = vao;
	tablet.cache.texture = texture;
	tablet.cache.width = width;
	tablet.cache.height = height;

	const size_t num_fixed_glyphs = (width * height);
	tablet.cache.max_num_glyphs = static_cast<int>(num_fixed_glyphs * 2); // allow twice as many glyphs to support layers

	GLuint vbo;

	const auto vert_loc = shader.attribute_loc(attribute_vert_pos);
	if (vert_loc >= 0)
	{
		IVec2 verts[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
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

	auto setup_glyph_data_attribute = [&shader](const char* attr, GLint size, GLenum type, GLboolean normalized, bool integer, const GLvoid* offset)
	{
		const auto attr_loc = shader.attribute_loc(attr);
		if (attr_loc >= 0)
		{
			if (integer)
			{
				glVertexAttribIPointer(attr_loc, size, type, sizeof(GlyphData), offset);
			}
			else
			{
				glVertexAttribPointer(attr_loc, size, type, normalized, sizeof(GlyphData), offset);
			}
			glEnableVertexAttribArray(attr_loc);
			glVertexAttribDivisor(attr_loc, 1);
		}
		else
		{
			printf("Cannot find shader attribute: %s\n", attr);
		}
	};

	setup_glyph_data_attribute(attribute_color1, 4, GL_UNSIGNED_BYTE, GL_TRUE, false, 0);
	setup_glyph_data_attribute(attribute_color2, 4, GL_UNSIGNED_BYTE, GL_TRUE, false, (void*)offsetof(GlyphData, color2));
	
	setup_glyph_data_attribute(attribute_page, 1, GL_UNSIGNED_BYTE, GL_FALSE, true, (void*)offsetof(GlyphData, page));
	setup_glyph_data_attribute(attribute_code, 1, GL_UNSIGNED_BYTE, GL_FALSE, true, (void*)offsetof(GlyphData, code));


	uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	const auto atlas_loc = shader.uniform_loc(uniform_atlas);
	if (atlas_loc >= 0)
	{
		glUseProgram(static_cast<GLuint>(shader_id));
		glUniform1i(atlas_loc, 0);
	}
	else
	{
		printf("Cannot find uniform: %s\n", uniform_atlas);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);


	// create vao for screen
	GLuint vao_screen;
	glGenVertexArrays(1, &vao_screen);
	glBindVertexArray(vao_screen);

	tablet.cache.vao_screen = vao_screen;
	
	const auto& screen_shader_cache = screen_shader_caches[screen_shader_cache_idx];
	if (screen_shader_cache.param_vert >= 0)
	{
		const float scale = 1.f;
		const float half_w = width * scale * 0.5f;
		const float half_h = height * scale * 0.5f;
		Vec3 verts[] = 
		{ 
			{ -half_w, -half_h, 0 }, { half_w, -half_h, 0 }, 
			{ half_w, half_h, 0 }, { -half_w, half_h, 0 } 
		};
		GLuint vbo_screen;
		glGenBuffers(1, &vbo_screen);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_screen);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		glVertexAttribPointer(screen_shader_cache.param_vert, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(screen_shader_cache.param_vert);
	}
	else
	{
		printf("Cannot find shader attribute: %s\n", attribute_vert_pos);
	}

	if (screen_shader_cache.param_uv >= 0)
	{
		float uvs[] = { 0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f };
		GLuint vbo_uv;
		glGenBuffers(1, &vbo_uv);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
		glVertexAttribPointer(screen_shader_cache.param_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(screen_shader_cache.param_uv);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	
	if (screen_shader_cache.param_texture >= 0)
	{
		glUseProgram(static_cast<GLuint>(screen_shader_id));
		glUniform1i(screen_shader_cache.param_texture, 0);
	}
	else
	{
		printf("Cannot find uniform: %s\n", uniform_texture);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);


	// create frame buffer for rendering to texture
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// TODO: rt texture size can become really big if width, height were set to real large?
	int glyph_w = 20; // TODO: read glyph pixel size from data (or based on texture size)
	int glyph_h = 20;
	int rt_w = width * glyph_w;
	int rt_h = height * glyph_h;

	// generate texture
	GLuint texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rt_w, rt_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0); 

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, rt_w, rt_h);  
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("tablet framebuffer is not ready!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

	tablet.cache.fbo = fbo;
	tablet.cache.rt_texture = texColorBuffer;
	tablet.cache.rt_width = rt_w;
	tablet.cache.rt_height = rt_h;

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

const TabletScreenShaderCache& TabletStore::screen_shader_cache(Id tablet_id) const
{
	const auto shader_idx = tablets[tablet_id].screen_shader_cache_idx;
	return screen_shader_caches[shader_idx];
}

namespace renderer
{
	void draw(const TabletStore& store, const TabletDrawStream& stream)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

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
			const auto& screen_shader_cache = store.screen_shader_cache(item.tablet_id);

			// render the tablet to texture
			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(tablet_cache.fbo));

			glViewport(0, 0, tablet_cache.rt_width, tablet_cache.rt_height);

			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// glEnable(GL_DEPTH_TEST);
			// glDepthFunc(GL_LESS);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tablet_cache.texture));

			// use shader
			glUseProgram(static_cast<GLuint>(shader_cache.shader_id));
			
			// set mvp
			//glUniformMatrix4fv(shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());
			glUniform2i(shader_cache.param_dims, tablet_cache.width, tablet_cache.height);

			glBindVertexArray(static_cast<GLuint>(tablet_cache.vao));

			asserts(tablet_cache.width * tablet_cache.height + item.extra_coords.size() == item.glyphs.size());
			const auto num_glyphs = std::min(static_cast<size_t>(tablet_cache.max_num_glyphs), item.glyphs.size());
			const auto num_fixed_glyphs = (tablet_cache.width * tablet_cache.height);

			// copy in extra coordinates
			if (!item.extra_coords.empty())			
			{
				glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.coord_buffer));
				glBufferSubData(GL_ARRAY_BUFFER, num_fixed_glyphs * sizeof(IVec2), item.extra_coords.size() * sizeof(IVec2), item.extra_coords.data());
			}

			// copy in the glyph data
			glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.glyph_buffer));
			glBufferSubData(GL_ARRAY_BUFFER, 0, item.glyphs.size() * sizeof(GlyphData), item.glyphs.data());

			// Draw all glyphs instanced
			// TODO: another way to draw this is to draw a quad, and have the pixel shader 
			// fill in the content based on uv
			// 6 because of two triangles, see above indices in add_tablet()
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(num_glyphs));
			// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);


			// draw quad on screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tablet_cache.rt_texture));

			// use shader
			glUseProgram(static_cast<GLuint>(screen_shader_cache.shader_id));
			
			// set mvp
			glUniformMatrix4fv(screen_shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());

			// draw elements
			glBindVertexArray(static_cast<GLuint>(tablet_cache.vao_screen));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}