#pragma once

#include "engine/math_types.h"
#include "engine/id.h"
#include "engine/viewpoint.h"
#include "engine/shader.h"
#include "engine/tablet.h"
#include "engine/texture.h"
#include "engine/window.h"
#include "engine/array.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include <vector>
#include <memory>
#include <chrono>


class TabletTestModel
{
public:
	struct TabletItem
	{
		// Id tablet_id;
		Pose pose;
		int width;
		int height;

		std::vector<IVec2> extra_coords;
		SimpleArray<GlyphData> glyphs;
	};

	Pose cam_pose;
	std::vector<TabletItem> tablets;

};

class TabletTestStore
{
public:
	Id tablet_shader;
	Id tablet_screen_shader;
	Texture atlas_texture;
	// TabletStore tablet_store;
};

class TabletTestApp
{
public:
	TabletTestApp();
	void update();
	bool ended();
	void present(const Context& ctx);
	
public:
	TabletTestModel model;
	TabletTestStore store;
};

