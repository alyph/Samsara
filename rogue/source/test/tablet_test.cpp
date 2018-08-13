
#include "tablet_test.h"
#include "graphics/renderer.h"
#include "math/math_utils.h"
#include <cstdlib>
#include <ctime>

TabletTestApp::TabletTestApp()
{
	// set a seed
	std::srand(std::time(nullptr));

	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = "Mesh Viewer";
	window = Window::create(params);

	renderer = std::make_unique<TabletTestRenderer>(*window);


	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	store.tablet_shader = Shader::create(desc);

	int width = 40;
	int height = 40;

	const auto id = store.tablet_store.add_tablet(width, height, store.tablet_shader);

	// create a item
	model.cam_pose = make_lookat(Vec3{0, 0, -60}, Vec3{0, 0, 0}, Vec3{0, 1, 0});
	model.tablets = { { id, Pose{}, width, height, {}, std::vector<GlyphData>(width * height) } };

	start_time = std::chrono::system_clock::now();
}

static uint8_t rand_byte()
{
	return (std::rand() & 0xff);
}

void TabletTestApp::update()
{
	window->poll_events();

	uint8_t c = 0;

	// randomize all glyphs
	for (auto& tablet_item : model.tablets)
	{
		for (auto& glyph : tablet_item.glyphs)
		{
			glyph.color1.r = rand_byte();
			glyph.color1.g = rand_byte();
			glyph.color1.b = rand_byte();
			glyph.color1.a = 255;
			glyph.code = rand_byte();
		}
	}

	// render stuff out
	renderer->render(store, model);

	// present
	window->present();

	// TODO: reuse this code
	frame_count++;
	auto now = std::chrono::system_clock::now();
	if ((now - start_time) >= std::chrono::duration<double>(1.0))
	{
		printf("-- fps: %f\n", frame_count / (std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time).count()));
		start_time = now;
		frame_count = 0;
	}
}

bool TabletTestApp::ended()
{
	return window->should_close();
}


TabletTestRenderer::TabletTestRenderer(Window& window)
{
	// TODO: window asepct may change at runtime.
	vp.projection = make_perspective(to_rad(60.f), window.aspect(), 0.1f, 100.f);
}

void TabletTestRenderer::render(const TabletTestStore& store, const TabletTestModel& model)
{
	vp.pose = model.cam_pose;
	const auto mat_vp = calc_mat_vp(vp);

	stream.items.resize(model.tablets.size());
	for (size_t i = 0; i < model.tablets.size(); i++)
	{
		auto& tablet_model = model.tablets[i];
		auto& item = stream.items[i];
		item.tablet_id = tablet_model.tablet_id;
		item.mvp = (mat_vp * to_mat44(tablet_model.pose));
		item.extra_coords = tablet_model.extra_coords;
		item.glyphs = tablet_model.glyphs;
	}

	// clear
	renderer::clear(Color{});

	// draw meshes
	renderer::draw(store.tablet_store, stream);
}


