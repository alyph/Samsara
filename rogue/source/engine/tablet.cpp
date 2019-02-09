
#include "tablet.h"
#include "shader.h"
#include "math_types.h"
#include "assertion.h"
#include "viewport.h"
#include "singleton.h"
#include "engine.h"
#include "easy/profiler.h"
#include <cstddef>
#include <algorithm>
#include <GL/glew.h>
#include <cmath>

namespace attrs
{
	Attribute<Id> quad_shader{null_id};
	Attribute<SimpleArray<GlyphData>> glyphs{SimpleArray<GlyphData>{}};
}

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

struct TabletCache
{
	Id vao{};
	Id vao_screen{};
	Id vert_buffer;
	Id coord_buffer{};
	Id glyph_buffer{};
	Id texture{};
	Id fbo{};
	Id rt_texture{};
	int width{};
	int height{};
	int max_num_glyphs{};
	int rt_width{};
	int rt_height{};

	Id shader_id{};
	Id quad_shader_id{};
	int param_dims{};
	// TODO: some of these params not needed
	int param_mvp{};
	int param_texture{};
	int param_vert{};
	int param_uv{};
};


struct TabletGlobals
{
	Id last_rendered_frame{};
	size_t next_tablet_index{};
	std::vector<TabletCache> tablet_caches;
};

SingletonHandle<TabletGlobals> tablet_globals;

// Id add_tablet(int width, int height, Id texture, Id shader, Id screen_shader)
// {
// 	return engine().singletons.get(tablet_globals).store.add_tablet(width, height, texture, shader, screen_shader);
// }

// Id TabletStore::add_tablet(int width, int height, Id texture, Id shader, Id screen_shader)
static void create_tablet_cache(TabletCache& cache, int width, int height, Id texture, Id shader, Id quad_shader)
{
	asserts(shader && quad_shader, "tablet shader must be valid.");
	asserts(width > 0 && height > 0, "tablet cannot be empty sized");
	asserts(texture, "tablet requires valid texture to render");

	// find or create a shader cache
	const auto shader_id = shader;

	cache.shader_id = shader_id;
	//cache.param_mvp = shader.uniform_loc(uniform_mvp);
	cache.param_dims = shader_uniform_loc(shader, uniform_dims);

	const auto screen_shader_id = quad_shader;
	cache.quad_shader_id = screen_shader_id;
	cache.param_mvp = shader_uniform_loc(quad_shader, uniform_mvp);
	cache.param_texture = shader_uniform_loc(quad_shader, uniform_texture);
	cache.param_vert = shader_attribute_loc(quad_shader, attribute_vert_pos);
	cache.param_uv = shader_attribute_loc(quad_shader, attribute_uv);

	// create vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	cache.vao = vao;
	cache.texture = texture;
	cache.width = width;
	cache.height = height;

	const size_t num_fixed_glyphs = (width * height);
	cache.max_num_glyphs = static_cast<int>(num_fixed_glyphs * 2); // allow twice as many glyphs to support layers

	// TODO: can maybe share verts between tablets
	GLuint vbo;
	const auto vert_loc = shader_attribute_loc(shader, attribute_vert_pos);
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
	glBufferData(GL_ARRAY_BUFFER, cache.max_num_glyphs * sizeof(IVec2), nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_fixed_glyphs * sizeof(IVec2), coords.data());
	cache.coord_buffer = vbo;

	const auto coord_loc = shader_attribute_loc(shader, attribute_coord);
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
	glBufferData(GL_ARRAY_BUFFER, cache.max_num_glyphs * sizeof(GlyphData), nullptr, GL_STREAM_DRAW);
	cache.glyph_buffer = vbo;

	auto setup_glyph_data_attribute = [&shader](const char* attr, GLint size, GLenum type, GLboolean normalized, bool integer, const GLvoid* offset)
	{
		const auto attr_loc = shader_attribute_loc(shader, attr);
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


	const auto atlas_loc = shader_uniform_loc(shader, uniform_atlas);
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

	cache.vao_screen = vao_screen;
	
	if (cache.param_vert >= 0)
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
		glVertexAttribPointer(cache.param_vert, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(cache.param_vert);
	}
	else
	{
		printf("Cannot find shader attribute: %s\n", attribute_vert_pos);
	}

	if (cache.param_uv >= 0)
	{
		float uvs[] = { 0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f };
		GLuint vbo_uv;
		glGenBuffers(1, &vbo_uv);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
		glVertexAttribPointer(cache.param_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(cache.param_uv);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	
	if (cache.param_texture >= 0)
	{
		glUseProgram(static_cast<GLuint>(screen_shader_id));
		glUniform1i(cache.param_texture, 0);
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

	cache.fbo = fbo;
	cache.rt_texture = texColorBuffer;
	cache.rt_width = rt_w;
	cache.rt_height = rt_h;
}

// const TabletCache& TabletStore::tablet_cache(Id tablet_id) const
// {
// 	return tablets[tablet_id].cache;
// }

// const TabletShaderCache& TabletStore::shader_cache(Id tablet_id) const
// {
// 	const auto shader_idx = tablets[tablet_id].shader_cache_idx;
// 	return shader_caches[shader_idx];
// }

// const TabletScreenShaderCache& TabletStore::screen_shader_cache(Id tablet_id) const
// {
// 	const auto shader_idx = tablets[tablet_id].screen_shader_cache_idx;
// 	return screen_shader_caches[shader_idx];
// }

// namespace renderer
// {
// 	void draw(const TabletStore& store, const TabletDrawStream& stream)
// 	{
// 		GLint viewport[4];
// 		glGetIntegerv(GL_VIEWPORT, viewport);

// 		// TODO: revert states
// 		// opengl states
// 		// depth test
// 		glEnable(GL_DEPTH_TEST);
// 		glDepthFunc(GL_LESS);
// 		glEnable(GL_CULL_FACE);

// 		for (const auto& item : stream.items)
// 		{
// 			const auto& tablet_cache = store.tablet_cache(item.tablet_id);
// 			const auto& shader_cache = store.shader_cache(item.tablet_id);
// 			const auto& screen_shader_cache = store.screen_shader_cache(item.tablet_id);

// 			// render the tablet to texture
// 			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(tablet_cache.fbo));

// 			glViewport(0, 0, tablet_cache.rt_width, tablet_cache.rt_height);

// 			glClearColor(0.f, 0.f, 0.f, 0.f);
// 			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 			// glEnable(GL_DEPTH_TEST);
// 			// glDepthFunc(GL_LESS);

// 			glActiveTexture(GL_TEXTURE0);
// 			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tablet_cache.texture));

// 			// use shader
// 			glUseProgram(static_cast<GLuint>(shader_cache.shader_id));
			
// 			// set mvp
// 			//glUniformMatrix4fv(shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());
// 			glUniform2i(shader_cache.param_dims, tablet_cache.width, tablet_cache.height);

// 			glBindVertexArray(static_cast<GLuint>(tablet_cache.vao));

// 			asserts(tablet_cache.width * tablet_cache.height + item.extra_coords.size() == item.glyphs.size());
// 			const auto num_glyphs = std::min(static_cast<size_t>(tablet_cache.max_num_glyphs), item.glyphs.size());
// 			const auto num_fixed_glyphs = (tablet_cache.width * tablet_cache.height);

// 			// copy in extra coordinates
// 			if (!item.extra_coords.empty())			
// 			{
// 				glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.coord_buffer));
// 				glBufferSubData(GL_ARRAY_BUFFER, num_fixed_glyphs * sizeof(IVec2), item.extra_coords.size() * sizeof(IVec2), item.extra_coords.data());
// 			}

// 			// copy in the glyph data
// 			glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.glyph_buffer));
// 			glBufferSubData(GL_ARRAY_BUFFER, 0, item.glyphs.size() * sizeof(GlyphData), item.glyphs.data());

// 			// Draw all glyphs instanced
// 			// TODO: another way to draw this is to draw a quad, and have the pixel shader 
// 			// fill in the content based on uv
// 			// 6 because of two triangles, see above indices in add_tablet()
// 			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(num_glyphs));
// 			// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

// 			glBindBuffer(GL_ARRAY_BUFFER, 0);
// 			glBindVertexArray(0);
// 			glBindTexture(GL_TEXTURE_2D, 0);


// 			// draw quad on screen
// 			glBindFramebuffer(GL_FRAMEBUFFER, 0);
// 			glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);


// 			glActiveTexture(GL_TEXTURE0);
// 			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tablet_cache.rt_texture));

// 			// use shader
// 			glUseProgram(static_cast<GLuint>(screen_shader_cache.shader_id));
			
// 			// set mvp
// 			glUniformMatrix4fv(screen_shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());

// 			// draw elements
// 			glBindVertexArray(static_cast<GLuint>(tablet_cache.vao_screen));
// 			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
// 			glBindVertexArray(0);
// 			glBindTexture(GL_TEXTURE_2D, 0);
// 		}
// 	}
// }


static Render3dType render_tablet(const Frame& frame, Id elem_id, const Mat44& transform)
{
	EASY_FUNCTION();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	auto& globals = engine().singletons.get(tablet_globals);
	if (frame.frame_id != globals.last_rendered_frame)
	{
		globals.next_tablet_index = 0;
		globals.last_rendered_frame = frame.frame_id;
	}

	// const TabletStore& store = engine().singletons.get(tablet_globals).store;
	// const auto tablet_id = get_elem_attr(frame, elem_id, attrs::tablet_id);
	const auto& glyphs = get_elem_attr(frame, elem_id, attrs::glyphs);
	const auto width = std::lround(get_elem_attr(frame, elem_id, attrs::width));
	const auto height = std::lround(get_elem_attr(frame, elem_id, attrs::height));
	const auto& texture = get_elem_attr(frame, elem_id, attrs::texture);
	const auto& shader = get_elem_attr(frame, elem_id, attrs::shader);
	const auto& quad_shader = get_elem_attr(frame, elem_id, attrs::quad_shader);

	const auto cache_index = globals.next_tablet_index++;
	if (cache_index >= globals.tablet_caches.size())
	{
		auto& new_cache = globals.tablet_caches.emplace_back();
		create_tablet_cache(new_cache, width, height, texture, shader, quad_shader);
	}

	const auto& tablet_cache = globals.tablet_caches[cache_index];
	asserts(tablet_cache.width == width && tablet_cache.height == height);
	asserts(tablet_cache.texture == texture && tablet_cache.shader_id == shader && tablet_cache.quad_shader_id == quad_shader);

	// const auto& tablet_cache = store.tablet_cache(tablet_id);
	// const auto& shader_cache = store.shader_cache(tablet_id);
	// const auto& screen_shader_cache = store.screen_shader_cache(tablet_id);

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
	glUseProgram(static_cast<GLuint>(tablet_cache.shader_id));
	
	// set mvp
	//glUniformMatrix4fv(shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());
	glUniform2i(tablet_cache.param_dims, tablet_cache.width, tablet_cache.height);

	glBindVertexArray(static_cast<GLuint>(tablet_cache.vao));

	asserts(tablet_cache.width * tablet_cache.height == glyphs.size());
	const auto num_glyphs = std::min(static_cast<size_t>(tablet_cache.max_num_glyphs), glyphs.size());
	const auto num_fixed_glyphs = (tablet_cache.width * tablet_cache.height);

	// copy in extra coordinates
	// if (!item.extra_coords.empty())			
	// {
	// 	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.coord_buffer));
	// 	glBufferSubData(GL_ARRAY_BUFFER, num_fixed_glyphs * sizeof(IVec2), item.extra_coords.size() * sizeof(IVec2), item.extra_coords.data());
	// }

	// copy in the glyph data
	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.glyph_buffer));
	glBufferSubData(GL_ARRAY_BUFFER, 0, glyphs.size() * sizeof(GlyphData), glyphs.data());

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
	glUseProgram(static_cast<GLuint>(tablet_cache.quad_shader_id));
	
	// set mvp
	const auto& tablet_tf = get_elem_attr(frame, elem_id, attrs::transform);
	const auto& mvp = transform * tablet_tf;
	glUniformMatrix4fv(tablet_cache.param_mvp, 1, GL_FALSE, mvp.data());

	// draw elements
	glBindVertexArray(static_cast<GLuint>(tablet_cache.vao_screen));
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return Render3dType::sub_tree;
}

static const Id tablet_elem_type = register_elem_type([](ElementTypeSetup& setup)
{
	setup.set_name("tablet");
	setup.set_attr(attrs::renderer_3d, &render_tablet);
});

namespace elem
{
	Id tablet(const Context ctx)
	{
		return make_element(ctx, tablet_elem_type);
	}
}