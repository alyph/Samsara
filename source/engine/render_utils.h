#pragma once

#include <GL/glew.h>

enum class DepthFunc: int
{
	less = GL_LESS,
	less_equal = GL_LEQUAL,
};

enum class AlphaFactor: int
{
	one = GL_ONE,
	src_alpha = GL_SRC_ALPHA,
	one_minus_src_alpha = GL_ONE_MINUS_SRC_ALPHA,
};

struct AlphaBlendFunc
{
	AlphaFactor src_rgb = AlphaFactor::src_alpha;
	AlphaFactor src_a = AlphaFactor::src_alpha;
	AlphaFactor dest_rgb = AlphaFactor::one_minus_src_alpha;
	AlphaFactor dest_a = AlphaFactor::one_minus_src_alpha;
};

struct RenderState
{
	bool depth_test{};
	bool alpha_blend{};
	bool cull_face = true;
	DepthFunc depth_func = DepthFunc::less;
	AlphaBlendFunc alpha_func{};
};

extern RenderState current_render_state();
extern void push_render_state(const RenderState& state);
extern void pop_render_state();


