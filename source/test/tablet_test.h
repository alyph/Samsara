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

		std::vector<Vec2i> extra_coords;
		Array<GlyphData> glyphs;
	};

	Pose cam_pose;
	std::vector<TabletItem> tablets;

};

class TabletTestStore
{
public:
	Id tablet_shader;
	Id tablet_screen_shader;
	Id atlas_texture;
	// TabletStore tablet_store;
};

class TabletTestApp
{
public:
	TabletTestApp();
	void update(double dt);
	bool ended();
	void present(const Context& ctx);
	void shutdown() {}
	
public:
	TabletTestModel model;
	TabletTestStore store;
};

