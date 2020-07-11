#include "render_utils.h"
#include "assertion.h"

// TODO: make a fixed statck for this type of things
static constexpr const size_t render_state_stack_capacity = 8;
static RenderState render_state_stack[render_state_stack_capacity]{};
static size_t render_state_stack_size = 0;

static void apply_render_state(const RenderState& state)
{
	if (state.depth_test)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(static_cast<GLenum>(state.depth_func));
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (state.alpha_blend)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(
			static_cast<GLenum>(state.alpha_func.src_rgb),
			static_cast<GLenum>(state.alpha_func.dest_rgb),
			static_cast<GLenum>(state.alpha_func.src_a),
			static_cast<GLenum>(state.alpha_func.dest_a)
		);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (state.cull_face)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

RenderState current_render_state()
{
	return render_state_stack_size > 0? render_state_stack[render_state_stack_size-1] : RenderState{};
}

void push_render_state(const RenderState& state)
{
	asserts(render_state_stack_size < render_state_stack_capacity);
	render_state_stack[render_state_stack_size++] = state;
	apply_render_state(state);
}

void pop_render_state()
{
	asserts(render_state_stack_size > 0);
	render_state_stack_size--;
	apply_render_state(current_render_state());
}
