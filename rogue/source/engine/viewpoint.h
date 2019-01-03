#pragma once

#include "math_types.h"

enum class Handedness
{
	left,
	right,
};

class Viewpoint
{
public:
	Pose pose; // world pose, the viewpoint look toward +z, up is +y, left/right is x (depending on handedness)
	Mat44 projection{Mat44::identity()};
	Handedness handedness = Handedness::left;
};

inline Pose make_lookat(const Vec3& from, const Vec3& to, const Vec3& up)
{
	Mat33 m;
	m[2] = normal(to - from);
	m[0] = normal(cross(up, m[2]));
	m[1] = cross(m[2], m[0]);
	return Pose{ from, to_quat(m) };
}

inline Mat44 make_perspective(float fovx, float aspect, float near, float far)
{
	// http://www.songho.ca/opengl/gl_projectionmatrix.html
	Mat44 m;
	const float r_over_n = std::tan(fovx * 0.5f);
	const float n_over_r = 1.f / r_over_n;
	m[0][0] = n_over_r;
	m[1][1] = n_over_r * aspect;
	m[2][2] = (far + near) / (far - near);
	m[2][3] = 1.f;
	m[3][2] = (-2.f * far * near) / (far - near);
	return m;
}

inline Mat44 calc_mat_vp(const Viewpoint& vp)
{
	// TODO: right handed
	// TODO: to other types of NDC (normalized device coordinates)
	// e.g. in d3d and vulkan z is in [0, 1]
	return vp.projection * to_mat44(inverse(vp.pose));
}



