#pragma once

#include "engine/math_types.h"
#include "engine/id.h"
#include "engine/viewpoint.h"
#include "engine/shader.h"
#include "engine/tablet.h"
#include "engine/texture.h"
#include "engine/window.h"
#include "engine/buffer.h"
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
		Id tablet_id;
		Pose pose;
		int width;
		int height;

		std::vector<IVec2> extra_coords;
		Buffer<GlyphData> glyphs;
	};

	Pose cam_pose;
	std::vector<TabletItem> tablets;

};

class TabletTestStore
{
public:
	Shader tablet_shader;
	Shader tablet_screen_shader;
	Texture atlas_texture;
	// TabletStore tablet_store;
};

class TabletTestRenderer
{
public:
	TabletTestRenderer(Window& window);
	//void render(const TabletTestStore& store, const TabletTestModel& model);

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
	void present(const Context& ctx);
	
private:
	EngineData engine;

	std::unique_ptr<Window> window;
	Presenter presenter;
	std::unique_ptr<TabletTestRenderer> renderer;

	TabletTestModel model;
	TabletTestStore store;

	std::chrono::system_clock::time_point start_time;
	int frame_count{};
};

