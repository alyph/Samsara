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

namespace attrs
{
	extern Attribute<Viewpoint> viewpoint;
	extern Attribute<Renderer3dFunc> renderer_3d;
}

namespace elem
{
	extern Id viewport(const Context ctx);
}

