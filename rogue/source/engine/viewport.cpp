
#include "viewport.h"
#include "presenter.h"
#include "easy/profiler.h"
#include <GL/glew.h>
#include <cmath>

namespace attrs
{
	Attribute<Viewpoint> viewpoint{Viewpoint{}};
	Attribute<Renderer3dFunc> renderer_3d{nullptr};
}

static void render_viewport(const Frame& frame, Id elem_id)
{
	EASY_FUNCTION();

	const auto width = std::lround(get_elem_attr_or_assert(frame, elem_id, attrs::width));
	const auto height = std::lround(get_elem_attr_or_assert(frame, elem_id, attrs::height));
	glViewport(0, 0, width, height);

	const auto color = get_elem_attr_or_default(frame, elem_id, attrs::background_color);
	glClearColor(color.r, color.g, color.b, color.a);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	const auto first_child = get_first_child(frame, elem_id);
	if (first_child)
	{
		const auto vp = get_elem_attr_or_default(frame, elem_id, attrs::viewpoint);
		const auto mat_vp = calc_mat_vp(vp);

		std::vector<Id> elem_stack;
		std::vector<Mat44> tf_stack;
		elem_stack.push_back(first_child);
		tf_stack.push_back(mat_vp);

		// TODO: configurable and maybe per sub tree configurable
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		while (!elem_stack.empty())
		{
			Id curr_elem_id = elem_stack.back();

			// render
			auto renderer = get_elem_attr_or_default(frame, curr_elem_id, attrs::renderer_3d);
			bool sub_tree_processed = false;
			if (renderer)
			{
				if (renderer(frame, curr_elem_id, tf_stack.back()) == Render3dType::sub_tree)
				{
					sub_tree_processed = true;
				}
			}

			// depth-first visit
			const Id child = (sub_tree_processed ? null_id : get_first_child(frame, curr_elem_id));
			if (child)
			{
				const auto tf = get_elem_attr_or_default(frame, curr_elem_id, attrs::transform);
				elem_stack.push_back(child);
				tf_stack.push_back(tf_stack.back() * tf);
			}
			else
			{
				do
				{
					const Id sibling = get_next_sibling(frame, curr_elem_id);
					if (sibling)
					{
						elem_stack.back() = sibling;
						break;
					}

					elem_stack.pop_back();
					tf_stack.pop_back();
				} 
				while (!elem_stack.empty());
			}
		}
	}
}

// static Id register_viewport_elem_type()
// {
// 	Id type_id = register_elem_type("viewport");
// 	set_elem_type_attr(type_id, attrs::renderer, &render_viewport);
// 	return type_id;
// }

// static const Id viewport_elem_type = register_viewport_elem_type();

static const Id viewport_elem_type = register_elem_type([](ElementTypeSetup& setup)
{
	setup.set_name("viewport");
	setup.set_attr(attrs::renderer, &render_viewport);
});

namespace elem
{
	Id viewport(const Context ctx)
	{
		return make_element(ctx, viewport_elem_type);
	}
}