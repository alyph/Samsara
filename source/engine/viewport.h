#pragma once

#include "math_types.h"
#include "viewpoint.h"
#include "presenter.h"

enum class Render3dType
{
	single,
	sub_tree,
};

using Renderer3dFunc = Render3dType(*)(const Frame& frame, Id elem_id, const Mat44& transform);
//using Raycaster3dFunc = Id(*)(const Frame& frame, Id elem_id, const Mat44& transform, const Vec3& start, const Vec3& dir, float dist, float& out_hit_dist);

// x, y, z in NDC (-1, 1)
using Raycaster3dFunc = RaycastResult(*)(const Frame& frame, Id elem_id, const Mat44& transform, double x, double y);

namespace attrs
{
	extern Attribute<Viewpoint> viewpoint;
	extern Attribute<Renderer3dFunc> renderer_3d;
	extern Attribute<Raycaster3dFunc> raycaster_3d;
}

namespace elem
{
	extern Id viewport(const Context ctx);
}

struct TriangleRaycastResult
{
	bool hit{};
	size_t hit_triangle_idx{};
	double z{}; // z in NDC (-1, 1)
	float w0{}, w1{}, w2{}; // barycentric coordinates
};
// TODO: use array view
// x, y in NDC (-1, 1)
extern TriangleRaycastResult raycast_triangles(size_t num_triangles, const Vec3* verts, const uint32_t* indices, const Mat44& transform, double x, double y);
inline DVec2 calc_ndc(const DVec2& pixel_coord, double vp_width, double vp_height);


inline DVec2 calc_ndc(const DVec2& pixel_coord, double vp_width, double vp_height)
{
	// TODO: maybe should take left, top too
	double ndc_x = (pixel_coord.x / vp_width) * 2.0 - 1.0;
	double ndc_y = (pixel_coord.y / vp_height) * -2.0 + 1.0;
	return {ndc_x, ndc_y};
}
