
#include "viewport.h"
#include "presenter.h"
#include "easy/profiler.h"
#include "math_utils.h"
#include <GL/glew.h>
#include <cmath>

namespace attrs
{
	Attribute<Viewpoint> viewpoint{Viewpoint{}};
	Attribute<Renderer3dFunc> renderer_3d{nullptr};
	Attribute<Raycaster3dFunc> raycaster_3d{nullptr};
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

static Id raycast_viewport(const Frame& frame, Id elem_id, double x, double y, double& out_z)
{
	const auto width = get_elem_attr_or_assert(frame, elem_id, attrs::width);
	const auto height = get_elem_attr_or_assert(frame, elem_id, attrs::height);
	const auto& vp = get_elem_attr_or_default(frame, elem_id, attrs::viewpoint);

	if (x > width || y > height)
	{
		return null_id; // outside of the viewport
	}

	// find out ndc for x, y
	// TODO: do we need add 0.5 to calculate based on the center of the pixel?
	double ndc_x = (x / width) * 2.0 - 1.0;
	double ndc_y = (y / height) * -2.0 + 1.0;	

	std::vector<Mat44> tf_stack;
	tf_stack.push_back(calc_mat_vp(vp));

	// loop through all sub elements and do raycast if provided, push in model view transform
	Id closest_hit_elem = null_id;
	double closest_hit_z = 2.0;

	const auto this_depth = frame.elements[id_to_index(elem_id)].depth;
	Id child_id = elem_id + 1;
	Id last = get_last_in_subtree(frame, elem_id);
	while (child_id <= last)
	{
		const auto& elem = frame.elements[id_to_index(child_id)];

		const size_t num_parent_tfs = (elem.depth - this_depth);

		// pop all tfs that are equal or lower depth, keep the ancestors' tfs 
		// so we don't need calculate them again
		if (tf_stack.size() > num_parent_tfs)
			tf_stack.resize(num_parent_tfs);

		auto raycaster = get_elem_attr_or_default(frame, child_id, attrs::raycaster_3d);
		if (raycaster)
		{
			// build up the tfs
			if (num_parent_tfs > tf_stack.size())
			{
				const int old_size = (int)tf_stack.size();
				tf_stack.resize(num_parent_tfs);

				Id curr_elem_id = child_id;
				for (int i = (int)tf_stack.size() - 1; i >= old_size; i--)
				{
					curr_elem_id = get_parent(frame, curr_elem_id);
					tf_stack[i] = get_elem_attr_or_default(frame, curr_elem_id, attrs::transform); // store local transform first
				}

				for (int i = old_size; i < tf_stack.size(); i++)
				{
					tf_stack[i] = tf_stack[i-1] * tf_stack[i];
				}
			}

			const auto& self_tf = get_elem_attr_or_default(frame, child_id, attrs::transform);

			double hit_z{};
			Id hit_elem = raycaster(frame, child_id, tf_stack.back() * self_tf, ndc_x, ndc_y, hit_z);
			if (hit_elem && hit_z < closest_hit_z)
			{
				closest_hit_elem = hit_elem;
				closest_hit_z = hit_z;
			}

			// skip the whole sub tree
			// TODO: for now all raycasters handle the whole sub tree
			child_id = get_last_in_subtree(frame, child_id) + 1;
		}
		else
		{
			child_id++;
		}
	}

	return closest_hit_elem;
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
	setup.set_attr(attrs::raycaster, &raycast_viewport);
});

namespace elem
{
	// TODO: maybe make this inline and extern the viewport_elem_type
	Id viewport(const Context ctx)
	{
		return make_element(ctx, viewport_elem_type);
	}
}


TriangleRaycastResult raycast_triangles(size_t num_triangles, const Vec3* verts, const uint32_t* indices, const Mat44& transform, double x, double y)
{
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage

	TriangleRaycastResult result;

	// this will be as if we are rasterizing the triangle, except only for 1 point defined by x, y
	for (size_t i = 0; i < num_triangles; i++)
	{
		auto v0 = transform * to_vec4(verts[indices[i*3]], 1.f);
		auto v1 = transform * to_vec4(verts[indices[i*3+1]], 1.f);
		auto v2 = transform * to_vec4(verts[indices[i*3+2]], 1.f);
		const float eps = 1e-6f;
		// if something very close to the eye or behind in perspective, it becomes unstable so just skip
		if (v0.w <= eps || v1.w <= eps || v2.w <= eps) continue; 
		v0 /= v0.w;
		v1 /= v1.w;
		v2 /= v2.w;

		const Vec2 v0_2d{v0.x, v0.y};
		const Vec2 v1_2d{v1.x, v1.y};
		const Vec2 v2_2d{v2.x, v2.y};
		const Vec2 p{(float)x, (float)y};

		// triangle widing is counter clock-wise
		// and in NDC, x points to right, y points to up
		float w2 = cross((v1_2d - v0_2d), (p - v0_2d));
		float w0 = cross((v2_2d - v1_2d), (p - v1_2d));
		float w1 = cross((v0_2d - v2_2d), (p - v2_2d));
		if (w0 < 0.f || w1 < 0.f || w2 < 0.f) continue; // not inside
		
		const float w = (w0 + w1 + w2);
		if (w <= 0.f) continue; // triangle is a line

		w0 /= w;
		w1 /= w;
		w2 /= w;

		const float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;
		const float z_clip = 1.f + eps;
		if (z >= -z_clip && z <= z_clip && (!result.hit || z < result.z))
		{
			result.hit = true;
			result.hit_triangle_idx = i;
			result.w0 = w0;
			result.w1 = w1;
			result.w2 = w2;
			result.z = z;
		}
	}

	return result;
}
