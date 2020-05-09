
#include "tablet.h"
#include "shader.h"
#include "math_types.h"
#include "assertion.h"
#include "viewport.h"
#include "singleton.h"
#include "engine.h"
#include "array.h"
#include "easy/profiler.h"
#include <cstddef>
#include <algorithm>
#include <GL/glew.h>
#include <cmath>

namespace attrs
{
	Attribute<Id> quad_shader{null_id};
	Attribute<ArrayView<GlyphData>> glyphs{ArrayView<GlyphData>{}};
}

static constexpr const char* uniform_mvp = "MVP";
static constexpr const char* uniform_tablet_dims = "TabletDims";
static constexpr const char* uniform_atlas = "Atlas";
static constexpr const char* uniform_texture = "Texture";
static constexpr const char* attribute_vert_pos = "VertexPos";
static constexpr const char* attribute_coord = "Coordinates";
static constexpr const char* attribute_dims = "Dimensions";
static constexpr const char* attribute_color1 = "Color1";
static constexpr const char* attribute_color2 = "Color2";
// static constexpr const char* attribute_page = "Page";
static constexpr const char* attribute_code = "Code";
static constexpr const char* attribute_uv = "UV";

static constexpr const GLenum atlas_target = GL_TEXTURE_2D_ARRAY;

struct TabletCache
{
	Id vao{};
	Id vao_screen{};
	Id vert_buffer;
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

struct TabletElemWorker
{
	Id id{};
	TabletLayout layout;
	int child_x{}, child_y{}, child_max_w{}, child_max_h{}; // potential child zone for current child if pushed, or next one if not pushed
	bool has_loose_children{};
};

struct TabletWorkState
{
	ArrayTemp<TabletElemWorker> finalizing_stack;
};

namespace attrs
{
	// internal state attribute used during a construction of a frame as well as rendering
	Attribute<TabletWorkState> tablet_work_state{TabletWorkState{}};
	Attribute<TabletRenderBuffer> tablet_render_buffer{TabletRenderBuffer{}};
	Attribute<TabletLayout> tablet_layout{TabletLayout{}};
}

Vec2 calc_tablet_size(int width, int height, Id texture)
{
	// TODO: probably need a better texture inteface to read these values
	GLint tex_w{}, tex_h{};
	glBindTexture(atlas_target, static_cast<GLuint>(texture));
	glGetTexLevelParameteriv(atlas_target, 0, GL_TEXTURE_WIDTH, &tex_w);
	glGetTexLevelParameteriv(atlas_target, 0, GL_TEXTURE_HEIGHT, &tex_h);
	glBindTexture(atlas_target, 0);

	return Vec2{ (float)width, (float)height * tex_h / tex_w };
}

extern TabletRenderBuffer& access_tablet_render_buffer_and_layout(const Context& context, TabletLayout& out_layout)
{
	Id tablet_elem_id = get_section_root(context);
	asserts(tablet_elem_id);
	finalize(context);
	out_layout = get_elem_attr_or_assert(*context.frame, get_working_elem_id(context), attrs::tablet_layout);
	return get_mutable_elem_attr_or_assert(*context.frame, tablet_elem_id, attrs::tablet_render_buffer);
}

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
	cache.param_dims = shader_uniform_loc(shader, uniform_tablet_dims);

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

	setup_glyph_data_attribute(attribute_coord, 2, GL_INT, GL_FALSE, true, (void*)offsetof(GlyphData, coords));
	setup_glyph_data_attribute(attribute_dims, 2, GL_INT, GL_FALSE, true, (void*)offsetof(GlyphData, size));

	setup_glyph_data_attribute(attribute_color1, 4, GL_UNSIGNED_BYTE, GL_TRUE, false, (void*)offsetof(GlyphData, color1));
	setup_glyph_data_attribute(attribute_color2, 4, GL_UNSIGNED_BYTE, GL_TRUE, false, (void*)offsetof(GlyphData, color2));
	
	// setup_glyph_data_attribute(attribute_page, 1, GL_UNSIGNED_BYTE, GL_FALSE, true, (void*)offsetof(GlyphData, page));
	setup_glyph_data_attribute(attribute_code, 1, GL_UNSIGNED_SHORT, GL_FALSE, true, (void*)offsetof(GlyphData, code));


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

	// TODO: probably need a better texture inteface to read these values
	GLint tex_w{}, tex_h{};
	glBindTexture(atlas_target, static_cast<GLuint>(texture));
	glGetTexLevelParameteriv(atlas_target, 0, GL_TEXTURE_WIDTH, &tex_w);
	glGetTexLevelParameteriv(atlas_target, 0, GL_TEXTURE_HEIGHT, &tex_h);
	glBindTexture(atlas_target, 0);

	// create vao for screen
	GLuint vao_screen;
	glGenVertexArrays(1, &vao_screen);
	glBindVertexArray(vao_screen);

	cache.vao_screen = vao_screen;
	
	if (cache.param_vert >= 0)
	{
		const auto tablet_size = calc_tablet_size(width, height, texture);
		const float half_w = tablet_size.x / 2;
		const float half_h = tablet_size.y / 2;
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
	int glyph_w = tex_w / 16; // TODO: hard coded 16 x 16 page size here
	int glyph_h = tex_h / 16;
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
		printf("tablet framebuffer is not ready!\n");
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

static int compute_elem_self_height(const Frame& frame, Id elem_id, int width)
{
	int height = -1;

	// if text attribute is set, use it to determine height
	auto text = get_elem_defined_attr(frame, elem_id, attrs::text);
	if (text)
	{
		if (width > 0)
		{
			// TODO: utf8
			// TODO: text wrap
			height = ((int)text->size() + width - 1) / width;
		}
		else
		{
			height = 0; // if width is 0, no text can render so just make height 0 as well
		}
	}


	// TODO: image?
	// TODO: height should be determined by max of all factors

	return height;
}

inline bool layout_done(const TabletLayout& layout)
{
	// can left, top be outside of the bounds???
	return layout.left >= 0 && layout.top >= 0 && layout.width >= 0 && layout.height >= 0;
}

// render and at the same time finalize layout if not done
static void render_and_layout_common_element(TabletRenderBuffer& render_buffer, Frame& frame, Id elem_id, TabletLayout& layout, int max_width, int max_height)
{
	int x = layout.left;
	int y = layout.top;
	int& w = layout.width;
	int& h = layout.height;
	const auto back_color = to_color32(get_elem_attr_or_default(frame, elem_id, attrs::background_color));
	const auto fore_color = to_color32(get_elem_attr_or_default(frame, elem_id, attrs::foreground_color));

	bool render_back =  (back_color.a != 0);
	size_t back_glyph_idx{};
	if (render_back)
	{
		GlyphData back_glyph;
		back_glyph.coords.x = x;
		back_glyph.coords.y = y;
		back_glyph.color1 = back_color;
		back_glyph_idx = render_buffer.push_glyph(elem_id, back_glyph);
	}

	// draw text, predict width height based on text length, wrapping etc.
	if (auto text = get_elem_defined_attr(frame, elem_id, attrs::text))
	{
		const auto& str = *text;
		// TODO: provide a character iterator to avoid caching these things
		const auto str_size = str.size();
		const auto str_data = str.data();
		if (w < 0)
		{
			w = std::min((int)str_size, max_width);
		}
		if (w > 0)
		{
			if (h < 0)
			{
				// TODO: utf8
				// TODO: text wrap
				// TODO: overflow or respect max_height?
				h = (((int)str_size + w - 1) / w);
			}

			const int max_size = std::min((int)str_size, (w * h));
			for (int c = 0; c < max_size; c++)
			{
				// TODO: tab, new line, text wrap, utf8
				const char code = *(str_data + c);
				if (code != '\0' && code != ' ')
				{
					int line = (int)(c / w);
					GlyphData glyph;
					glyph.color2 = fore_color; // color1 is left to be 0s (transparent)
					glyph.code = code;
					glyph.coords.x = (x + (c % w));
					glyph.coords.y = (y + line);
					render_buffer.push_glyph(elem_id, glyph);
				}
			}
		}
	}
	else if (auto glyphs = get_elem_defined_attr(frame, elem_id, attrs::glyphs))
	{
		const auto num_glyphs = glyphs->size();
		if (w < 0) { w = std::min((int)num_glyphs, max_width); }
		if (w > 0)
		{
			if (h < 0) { h = (((int)num_glyphs + w - 1) / w); }
			const int max_size = std::min((int)num_glyphs, (w * h));
			for (int i = 0; i < max_size; i++)
			{
				const auto glyph_idx = render_buffer.push_glyph(elem_id, (*glyphs)[i]);
				auto& glyph = render_buffer.mutate_glyph(glyph_idx);
				glyph.coords.x += x;
				glyph.coords.y += x;
			}
		}
	}
	// TODO: image and other common elements
	
	// fill remaining missing layout
	// TODO: horizontal layout would be different
	if (h <= 0) // if height 0 or undefined, everything is 0
	{
		w = 0; h = 0;
	}
	else // otherwise, undefined width will be set to max width
	{
		if (w < 0) { w = max_width; }
	}

	// draw background (if background is not transparent) once size is determined
	if (render_back)
	{
		if (w > 0 && h > 0)
		{
			// updat the glyph to correct size
			auto& back_glyph = render_buffer.mutate_glyph(back_glyph_idx);
			back_glyph.size.x = w;
			back_glyph.size.y = h;
		}
		else
		{
			// if it's degenerated, nothing should have been drawn either, so safely pop the back glyph
			render_buffer.pop_glyph(back_glyph_idx);
		}
	}

	// all layout should be set now, save that in the attribute
	asserts(layout_done(layout));
	// is post attr now basicaclly a different bank to help minimizing insertion?
	set_elem_post_attr(frame, elem_id, attrs::tablet_layout, layout);
}

static void finalize_parent_after_structured_children(TabletRenderBuffer& render_buffer, Frame& frame, Id elem_id, Id before_child_id, TabletLayout& layout, int max_width, int max_height)
{
	// TODO: simplify code here with a Rect type
	bool bounds_set = false;
	TabletLayout bounds;
	Id child_id = get_first_child(frame, elem_id);
	Id end_id = (before_child_id ? before_child_id : -1);
	while (child_id && (child_id < end_id))
	{
		const auto& child_layout = get_elem_attr_or_assert(frame, child_id, attrs::tablet_layout);
		if (bounds_set)
		{
			bounds.width = std::max(bounds.left + bounds.width, child_layout.left + child_layout.width) - bounds.left;
			bounds.height = std::max(bounds.top + bounds.height, child_layout.top + child_layout.height) - bounds.top;
			bounds.left = std::min(bounds.left, child_layout.left);
			bounds.top = std::min(bounds.top, child_layout.top);
		}
		else
		{
			bounds = child_layout;
		}					
		child_id = get_next_sibling(frame, child_id);
	}

	// if bounds not set (no children), will take last chance in the render step next
	if (bounds_set)
	{
		if (layout.width < 0)
		{
			layout.left = bounds.left;
			layout.width = bounds.width;
		}
		if (layout.height < 0)
		{
			layout.top = bounds.top;
			layout.height = bounds.height;
		}
	}

	// render and finalize layout
	render_and_layout_common_element(render_buffer, frame, elem_id, layout, max_width, max_height);
}

static void finalize_tablet_elems(Frame& frame, Id root_elem_id, Id first_elem_id, Id last_elem_id)
{
	// NOTE: It's only safe to keep a pointer to these attribute values because we are not messing with the elements'
	// instance attribute buffer but only adding to the post attribute buffer
	TabletWorkState& state = get_mutable_elem_attr_or_assert(frame, root_elem_id, attrs::tablet_work_state);
	TabletRenderBuffer& render_buffer = get_mutable_elem_attr_or_assert(frame, root_elem_id, attrs::tablet_render_buffer);

	// empty indicate this is the first time calling, 
	// and the setup of the tablet itself is complete
	auto& stack = state.finalizing_stack;
	if (stack.empty())
	{
		TabletElemWorker worker;
		worker.id = root_elem_id;
		worker.child_max_w = worker.layout.width = get_elem_attr_or_assert(frame, root_elem_id, attrs::width).to_int();
		worker.child_max_h = worker.layout.height = get_elem_attr_or_assert(frame, root_elem_id, attrs::height).to_int();
		stack.push_back(worker);

		render_and_layout_common_element(render_buffer, frame, root_elem_id, worker.layout, worker.layout.width, worker.layout.height);
	}

	int tw = stack[0].layout.width;
	int th = stack[0].layout.height;

	Id curr_id = first_elem_id;
	while (curr_id <= last_elem_id || stack.size() > 1)
	{
		bool advance = true;
		const Id prev_id = stack.back().id;
		const auto& prev_elem = get_element(frame, prev_id);
		if (curr_id > last_elem_id) // at the end
		{
			// the processing on the node can't be completed if the tree is still open
			// (i.e. tree offset is 0, meaning more children can be added)
			// or not all children are included within the current processing range
			// (i.e. last child is > last_elem_id)
			// TODO: pop if the first child already not in range as this means it's a sub section root
			const bool all_children_processed =
				(prev_elem.tree_offset > 0 && 
				(prev_id + prev_elem.tree_offset - 1 <= last_elem_id || // either all descendants in range (cheap check)
				get_last_child(frame, prev_id) <= last_elem_id)); // or all immediate children in range (expensive check), for cases of sub sections within this tree
			if (all_children_processed) { advance = false; } // if all completed, pop
			else { break; } // otherwise end finalization and wait until next finalize cal
		}
		// current elem is not the child of last
		else if (get_element(frame, curr_id).depth <= prev_elem.depth)
		{
			advance = false; // pop last
		}

		if (advance)
		{
			// eval layout, render and push
			auto& parent = stack.back();
			TabletElemWorker worker;
			worker.id = curr_id;
			const auto placement = get_elem_attr_or_default(frame, curr_id, attrs::placement);
			const auto& left = get_elem_attr_or_default(frame, curr_id, attrs::left);
			const auto& top = get_elem_attr_or_default(frame, curr_id, attrs::top);
			const auto& width = get_elem_attr_or_default(frame, curr_id, attrs::width);
			const auto& height = get_elem_attr_or_default(frame, curr_id, attrs::height);

			if (placement == ElementPlacement::loose)
			{
				if (!parent.has_loose_children)
				{
					if (!layout_done(parent.layout))
					{
						// finalize parent layout now
						const auto& grand_parent = stack[stack.size() - 2];
						finalize_parent_after_structured_children(render_buffer, frame, parent.id, curr_id, parent.layout, grand_parent.child_max_w, grand_parent.child_max_h);
					}
					parent.has_loose_children = true;
				}
				auto& layout = worker.layout;
				layout.left = parent.layout.left + (left.undefined() ? 0 : left.to_int());
				layout.top = parent.layout.top + (top.undefined() ? 0 : top.to_int());
				layout.width = (width.undefined() ? -1 : width.to_int());
				layout.height = (height.undefined() ? -1 : height.to_int());
				worker.child_x = layout.left;
				worker.child_y = layout.top;
				worker.child_max_w = (layout.width < 0 ? (tw - layout.left) : layout.width);
				worker.child_max_h = (layout.height < 0 ? (th - layout.top) : layout.height);
			}
			else // ElementPlacement::structured
			{
				// all loose elems must come after the structured siblings
				asserts(!parent.has_loose_children);
				auto& layout = worker.layout;
				// TODO: top, left attributes
				worker.child_x = worker.layout.left = parent.child_x;
				worker.child_y = worker.layout.top = parent.child_y;
				// TODO: horizontal layout
				// TODO: % values
				// TODO: should always take >= value, maybe add a to_pos_int()
				layout.width = (width.undefined() ? -1 : width.to_int());
				layout.height = (height.undefined() ? -1 : height.to_int());
				worker.child_max_w = (layout.width < 0 ? parent.child_max_w : layout.width);
				worker.child_max_h = (layout.height < 0 ? parent.child_max_h : layout.height);
			}

			// render now if all layout is already done
			if (layout_done(worker.layout))
			{
				render_and_layout_common_element(render_buffer, frame, curr_id, worker.layout, parent.child_max_w, parent.child_max_h);
			}
			stack.push_back(worker);
			curr_id++;
		}
		else
		{
			auto& worker = stack.back();
			auto& parent = stack[stack.size() - 2];

			// if layout still not done, it must not have any loose children, finalize layout now
			if (!layout_done(worker.layout))
			{
				asserts(!worker.has_loose_children);
				finalize_parent_after_structured_children(render_buffer, frame, worker.id, null_id, worker.layout, parent.child_max_w, parent.child_max_h);
			}

			// update parent's child box if this is not a loose child
			// NOTE: elements always popped before the next sibling pushed, so if the parent is
			// marked with has loose children, then this child and some of the previous siblings
			// must be loose children 
			if (!parent.has_loose_children)
			{
				// only height is adjusted for vertical stacking
				// TODO: handle horizontal stacking and any other layout options
				// also need to handle pull down or pull right cases
				const auto occupied_height = std::min(worker.layout.top + worker.layout.height - parent.child_y, parent.child_max_h);
				parent.child_y += occupied_height;
				parent.child_max_h -= occupied_height;
			}

			// pop
			stack.pop_back();
		}
	}
}

#if 0
static void postprocess_tablet(Frame& frame, Id elem_id)
{
	EASY_FUNCTION();

	struct WorkItem
	{
		size_t parent_idx{};
		int width = -1, height = -1, left{}, top{};
	};

	size_t num_items = get_last_in_subtree(frame, elem_id) - elem_id + 1;
	std::vector<WorkItem> items{num_items};

	auto& root_item = items[0];
	root_item.width = std::lround(get_elem_attr_or_assert(frame, elem_id, attrs::width));
	root_item.height = std::lround(get_elem_attr_or_assert(frame, elem_id, attrs::height));

	// first pass
	// cache parent idx
	// set any defined attributes
	for (size_t i = 0; i < num_items; i++)
	{
		Id child_elem_id = get_first_child(frame, elem_id + i);
		while (child_elem_id)
		{
			items[child_elem_id - elem_id].parent_idx = i;
			child_elem_id = get_next_sibling(frame, child_elem_id);
		}
	}

	// backwards to init all that can be computed
	// and width based on children
	for (size_t i = num_items - 1; i > 0; i--) // skip the root as it has been "computed"
	{
		const auto curr_elem_id = elem_id + i;

		// set width if attr exists
		auto width = get_elem_defined_attr(frame, curr_elem_id, attrs::width);
		if (width)
		{
			items[i].width = std::max(std::lround(*width), 0l); // silent fail on negative width, clamp to 0
		}
		else // otherwise if any children has width set then take the max?
		{
			// TODO: some attr can determine width (e.g. image)
			// TODO: and some may influence but not determine width (e.g. text, maybe set max width??)

			int max_children_width = -1; // if nothing set, then set to -1 as if nothing happened
			Id child_elem_id = get_first_child(frame, curr_elem_id);
			while (child_elem_id)
			{
				max_children_width = std::max(items[child_elem_id - elem_id].width, max_children_width);
				child_elem_id = get_next_sibling(frame, child_elem_id);
			}

			items[i].width = max_children_width;
		}
		
		// TODO: margin, left, right offset etc. for this we need an outer size
	}

	// forward to compute all width
	for (size_t i = 1; i < num_items; i++) // again skip the root
	{
		// if still not set at this point, match size with the parent
		auto& item = items[i];
		if (item.width < 0)
		{
			item.width = items[item.parent_idx].width;
		}
		// TODO: margin, left, right offset etc.
		// TODO: % width		
	}

	// backwards to compute self height and height based on children
	for (size_t i = num_items - 1; i > 0; i--)
	{
		const auto curr_elem_id = elem_id + i;
		auto& item = items[i];

		// set height if attr exists
		auto height = get_elem_defined_attr(frame, curr_elem_id, attrs::height);
		if (height)
		{
			item.height = std::max(std::lround(*height), 0l); // silent fail on negative height, clamp to 0
		}
		else
		{
			item.height = compute_elem_self_height(frame, curr_elem_id, item.width);
			if (item.height < 0)
			{
				Id child_elem_id = get_first_child(frame, curr_elem_id);
				if (child_elem_id)
				{
					int total_children_height = 0;
					while (child_elem_id)
					{
						int child_height = items[child_elem_id - elem_id].height;
						if (child_height < 0)
						{
							total_children_height = -1;
							break;
						}
						total_children_height += child_height;
						child_elem_id = get_next_sibling(frame, child_elem_id);
					}
					item.height = total_children_height;
				}
			}
		}

		// TODO: margin, left, right offset etc.
		// TODO: % height
	}

	// forward to fill in all heights
	for (size_t i = 0; i < num_items; i++)
	{
		const auto curr_elem_id = elem_id + i;
		int num_children_undef_height = 0;
		int total_children_def_height = 0;
		Id child_elem_id = get_first_child(frame, curr_elem_id);
		while (child_elem_id)
		{
			const int child_height = items[child_elem_id - elem_id].height;
			if (child_height < 0) 
			{
				num_children_undef_height++;
			}
			else
			{
				total_children_def_height += child_height;
			}			
			child_elem_id = get_next_sibling(frame, child_elem_id);
		}

		if (num_children_undef_height > 0)
		{
			int remaining_height = std::max(items[curr_elem_id - elem_id].height - total_children_def_height, 0);
			int distr_child_height = (remaining_height / num_children_undef_height);
			int idx = 0;
			child_elem_id = get_first_child(frame, curr_elem_id);
			while (child_elem_id)
			{
				auto& child_item = items[child_elem_id - elem_id];
				if (child_item.height < 0) 
				{
					child_item.height = distr_child_height;
					if (idx == (num_children_undef_height - 1))
					{
						child_item.height += (remaining_height % num_children_undef_height);
					}
					idx++;
				}
				child_elem_id = get_next_sibling(frame, child_elem_id);
			}
		}
	}

	// pass to determine left, top
	for (size_t i = 0; i < num_items; i++)
	{
		const auto curr_elem_id = elem_id + i;
		const auto& parent_item = items[i];
		int top = parent_item.top;
		Id child_elem_id = get_first_child(frame, curr_elem_id);
		while (child_elem_id)
		{
			auto& child_item = items[child_elem_id - elem_id];
			child_item.left = parent_item.left; // TODO: padding, margin, left, right
			child_item.top = top;
			top += child_item.height;
			child_elem_id = get_next_sibling(frame, child_elem_id);
		}
	}

	// set the layout attribute
	for (size_t i = 0; i < num_items; i++)
	{
		const auto& item = items[i];
		TabletLayout layout;
		layout.left = item.left;
		layout.top = item.top;
		layout.width = item.width;
		layout.height = item.height;
		set_elem_post_attr(frame, elem_id + i, attrs::tablet_layout, layout);
	}
}
#endif

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

	const auto& root_layout = get_elem_attr_or_assert(frame, elem_id, attrs::tablet_layout);

	const int width = root_layout.width;
	const int height = root_layout.height;

	const auto& texture = get_elem_attr_or_assert(frame, elem_id, attrs::texture);
	const auto& shader = get_elem_attr_or_assert(frame, elem_id, attrs::shader);
	const auto& quad_shader = get_elem_attr_or_assert(frame, elem_id, attrs::quad_shader);
	const auto& render_buffer = get_elem_attr_or_assert(frame, elem_id, attrs::tablet_render_buffer);

	// create cache for the given tablet
	// TODO: match uuid of the element (not the elem_id since that's per frame)
	const auto cache_index = globals.next_tablet_index++;
	if (cache_index >= globals.tablet_caches.size())
	{
		auto& new_cache = globals.tablet_caches.emplace_back();
		create_tablet_cache(new_cache, width, height, texture, shader, quad_shader);
	}

	// TODO: recreate the cache if the dimensions or any other attributes changed
	const auto& tablet_cache = globals.tablet_caches[cache_index];
	asserts(tablet_cache.width == width && tablet_cache.height == height);
	asserts(tablet_cache.texture == texture && tablet_cache.shader_id == shader && tablet_cache.quad_shader_id == quad_shader);
		
	const auto& glyphs = render_buffer.glyphs;
	const size_t num_glyphs = glyphs.size();

	if (num_glyphs > 0)
	{
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
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(atlas_target, static_cast<GLuint>(tablet_cache.texture)); // texture for glyph atlas
		glUseProgram(static_cast<GLuint>(tablet_cache.shader_id)); // glyph shader;
		glUniform2i(tablet_cache.param_dims, tablet_cache.width, tablet_cache.height); // dimensions uniform
		glBindVertexArray(static_cast<GLuint>(tablet_cache.vao)); // tablet glyph vao
		glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.glyph_buffer));


		// make sure gl buffer has enough space for all glyphs
		if (num_glyphs > tablet_cache.max_num_glyphs)
		{
			glBufferData(GL_ARRAY_BUFFER, num_glyphs * 2 * sizeof(GlyphData), nullptr, GL_STREAM_DRAW);
		}

		// insert each block index into a temp array sorted by elem id
		// TODO: maybe more cache friendly if we just insert all block data in instead of just index?
		const auto& blocks = render_buffer.blocks;
		Array<size_t> sorted_block_indices{0, blocks.size(), temp_allocator()};
		for (size_t block_idx = 0; block_idx < blocks.size(); block_idx++)
		{
			const Id block_elem_id = blocks[block_idx].elem_id;
			size_t ins = sorted_block_indices.size();
			for (; ins > 0; ins--)
			{
				if (block_elem_id >= blocks[sorted_block_indices[ins-1]].elem_id) { break; }
			}
			sorted_block_indices.insert(ins, block_idx);
		}

		// loop over the block index array and copy data to gl
		// (if that's too slow, copy to temp array and then call glBufferSubData)
		size_t curr = 0;
		GLintptr glyph_buffer_offset = 0;
		while (curr < sorted_block_indices.size())
		{
			const auto begin = curr;
			size_t next = curr + 1;
			while (next < sorted_block_indices.size())
			{
				if (sorted_block_indices[next] != sorted_block_indices[curr] + 1) { break; }
				curr = next++;
			}
			const size_t begin_idx = sorted_block_indices[begin];
			const size_t end_idx = sorted_block_indices[curr];

			const size_t begin_glyph_idx = blocks[begin_idx].buffer_idx;
			const size_t end_glyph_idx = (end_idx + 1 < blocks.size()) ? blocks[end_idx+1].buffer_idx : glyphs.size();
			const size_t num_copy_glyphs = (end_glyph_idx - begin_glyph_idx);

			// glBufferSubData from begin to end
			glBufferSubData(GL_ARRAY_BUFFER, glyph_buffer_offset, num_copy_glyphs * sizeof(GlyphData), glyphs.data() + begin_glyph_idx);
			glyph_buffer_offset += num_copy_glyphs;
			curr++;
		}
		// Draw all glyphs instanced
		// TODO: another way to draw this is to draw a quad, and have the pixel shader 
		// fill in the content based on uv
		// 6 because of two triangles, see above indices in add_tablet()
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(num_glyphs));
		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// construct glyph data and upload
#if 0
	const auto num_fixed_glyphs = (tablet_cache.width * tablet_cache.height);
	SimpleArray<GlyphData> glyphs = alloc_simple_array<GlyphData>(num_fixed_glyphs, true);
	glyphs.zero();
	const Id last_elem_id = get_last_in_subtree(frame, elem_id);
	for (Id id = elem_id; id <= last_elem_id; id++)
	{
		const auto& layout = get_elem_attr_or_assert(frame, id, attrs::tablet_layout);
		// TODO: background color

		if (auto defined_glyphs = get_elem_defined_attr(frame, id, attrs::glyphs))
		{
			int src_x{0}, src_y{0}, dst_x{layout.left}, dst_y{layout.top};
			int w{layout.width}, h{layout.height};

			if (dst_x < 0) { int delta = -dst_x; src_x += delta; w -= delta; dst_x = 0; }
			if (dst_x + w > width) { w = width - dst_x;	}
			if (dst_y < 0) { int delta = -dst_y; src_y += delta; h -= delta; dst_y = 0; }
			if (dst_y + h > height) { h = height - dst_y; }

			if (w > 0 && h > 0)
			{
				const size_t num_glyphs = defined_glyphs->size();
				for (int y = 0; y < h; y++)
				{
					int src_start = y * layout.width + src_x;
					int dst_start = (dst_y + y) * width + dst_x;
					if (src_start + w <= num_glyphs)
					{
						memcpy(glyphs.data() + dst_start, defined_glyphs->data() + src_start, w * sizeof(GlyphData));
					}
					else
					{
						if (src_start < num_glyphs)
						{
							memcpy(glyphs.data() + dst_start, defined_glyphs->data() + src_start, (num_glyphs - src_start) * sizeof(GlyphData));
						}
						break;
					}					
				}
			}
		}
		else if (auto text = get_elem_defined_attr(frame, id, attrs::text))
		{
			const auto len = text->str().size();
			auto c_str = text->str().c_str();
			for (size_t i = 0; i < len; i++)
			{
				int line = (int)(i / layout.width); // TODO: tab, new line, text wrap, utf8
				int x = layout.left + (i % layout.width);
				int y = layout.top + line;

				if (x >= 0 && y >= 0 && x < width && y < height)
				{
					auto& glyph = glyphs[x + width * y];
					glyph.code = *(c_str + i);
					glyph.color1 = to_color32(get_elem_attr_or_default(frame, id, attrs::background_color));
					glyph.color2 = to_color32(get_elem_attr_or_default(frame, id, attrs::foreground_color));
				}		
			}
		}
	}
	
	const auto num_glyphs = std::min(static_cast<size_t>(tablet_cache.max_num_glyphs), glyphs.size());

	// copy in extra coordinates
	// if (!item.extra_coords.empty())			
	// {
	// 	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.coord_buffer));
	// 	glBufferSubData(GL_ARRAY_BUFFER, num_fixed_glyphs * sizeof(IVec2), item.extra_coords.size() * sizeof(IVec2), item.extra_coords.data());
	// }

	// copy in the glyph data
	// TODO: do we need the optimization of only uploading data that changed?
	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(tablet_cache.glyph_buffer));
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_glyphs * sizeof(GlyphData), glyphs.data());
#endif

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindTexture(atlas_target, 0);


		// draw quad on screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(tablet_cache.rt_texture));

		// use shader
		glUseProgram(static_cast<GLuint>(tablet_cache.quad_shader_id));
		
		// set mvp
		const auto& tablet_tf = get_elem_attr_or_default(frame, elem_id, attrs::transform);
		const auto& mvp = transform * tablet_tf;
		glUniformMatrix4fv(tablet_cache.param_mvp, 1, GL_FALSE, mvp.data());

		// draw elements
		glBindVertexArray(static_cast<GLuint>(tablet_cache.vao_screen));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	return Render3dType::sub_tree;
}

static RaycastResult raycast_tablet(const Frame& frame, Id elem_id, const Mat44& transform, double x, double y)
{
	const auto& root_layout = get_elem_attr_or_assert(frame, elem_id, attrs::tablet_layout);
	const int width = root_layout.width;
	const int height = root_layout.height;
	const auto& texture = get_elem_attr_or_assert(frame, elem_id, attrs::texture);

	const auto tablet_size = calc_tablet_size(width, height, texture);
	const float half_w = tablet_size.x / 2;
	const float half_h = tablet_size.y / 2;
	const Vec3 verts[] = 
	{
		{ -half_w, -half_h, 0 }, { half_w, -half_h, 0 },
		{ half_w, half_h, 0 }, { -half_w, half_h, 0 }
	};
	const uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
	const Vec2 uvs[] = { {0.f, 1.f}, {1.f, 1.f}, {1.f, 0.f}, {0.f, 0.f} };

	RaycastResult hit_result;
	const auto& result = raycast_triangles(2, verts, indices, transform, x, y);
	if (result.hit)
	{
		hit_result.point = {(float)x, (float)y, (float)result.z};

		const uint32_t i0 = indices[result.hit_triangle_idx*3];
		const uint32_t i1 = indices[result.hit_triangle_idx*3 + 1];
		const uint32_t i2 = indices[result.hit_triangle_idx*3 + 2];

		const Vec2 uv = uvs[i0] * result.w0 + uvs[i1] * result.w1 + uvs[i2] * result.w2;

		const int cx = std::clamp((int)std::floor(uv.x * width), 0, width-1);
		const int cy = std::clamp((int)std::floor(uv.y * height), 0, height-1);

		hit_result.uv = uv;
		hit_result.ruv = {uv.x * width, uv.y * height};
		hit_result.iuv = {cx, cy};

		// loop backwards to find the deepest hit descendent
		// TODO: may want to optimize such that we can skip the whole sub tree
		const Id last_elem_id = get_last_in_subtree(frame, elem_id);
		for (Id id = last_elem_id; id > elem_id; id--)
		{
			const auto& layout = get_elem_attr_or_assert(frame, id, attrs::tablet_layout);
			if (cx >= layout.left && cx < layout.left + layout.width &&
				cy >= layout.top && cy < layout.top + layout.height)
			{
				hit_result.hit_elem_id = id;
				break;
			}
		}

		// TODO: add an attribute for whether or not the parent should receive raycast hit
		// for now just default to no hit, if no children hit
		// if no child hit, tablet itself is hit
		// if (!hit_result.hit_elem_id) { hit_result.hit_elem_id = elem_id; }
	}

	return hit_result;
}

static const Id tablet_elem_type = register_elem_type([](ElementTypeSetup& setup)
{
	setup.set_name("tablet");
	setup.as_section_root();
	setup.set_attr(attrs::renderer_3d, &render_tablet);
	setup.set_attr(attrs::finalizer, &finalize_tablet_elems);
	setup.set_attr(attrs::raycaster_3d, &raycast_tablet);
});

namespace elem
{
	// TODO: maybe make this inline and extern the tablet_elem_type
	Id tablet(const Context ctx)
	{
		Id tablet_id = make_element(ctx, tablet_elem_type);

		// TODO: set self layout
		TabletWorkState work_state;
		work_state.finalizing_stack.alloc(0, 16);
		_attr(attrs::tablet_work_state, work_state);

		TabletRenderBuffer render_buffer;
		// TODO: ideally if we could store the last frame's allocated size in a persistent state, then we could inherit that size
		render_buffer.glyphs.alloc(0, 32 * 1024); // 32k preallocated buffer size
		render_buffer.blocks.alloc(0, 1024); // 1k blocks (roughly how many sub elements in the tablet)
		_attr(attrs::tablet_render_buffer, render_buffer);
		
		return tablet_id;
	}
}

