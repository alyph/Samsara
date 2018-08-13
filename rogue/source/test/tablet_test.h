#pragma once

#include "math/math_types.h"
#include "types/id.h"
#include "graphics/viewpoint.h"
#include "graphics/shader.h"
#include "graphics/tablet.h"
#include "window/window.h"
#include <vector>
#include <memory>
#include <chrono>


class TabletTestModel
{
public:
	struct TabletItem
	{
		Id tablet_id;
		Pose pose;
		int width;
		int height;

		std::vector<IVec2> extra_coords;
		std::vector<GlyphData> glyphs;
	};

	Pose cam_pose;
	std::vector<TabletItem> tablets;

};

class TabletTestStore
{
public:
	Shader tablet_shader;
	TabletStore tablet_store;
};

class TabletTestRenderer
{
public:
	TabletTestRenderer(Window& window);
	void render(const TabletTestStore& store, const TabletTestModel& model);

private:
	Viewpoint vp;
	TabletDrawStream stream;
};

class TabletTestApp
{
public:
	TabletTestApp();
	void update();
	bool ended();

private:
	std::unique_ptr<Window> window;
	std::unique_ptr<TabletTestRenderer> renderer;

	TabletTestModel model;
	TabletTestStore store;

	std::chrono::system_clock::time_point start_time;
	int frame_count{};
};

