
#include "tablet_test.h"
#include "engine/renderer.h"
#include "engine/math_utils.h"
#include "engine/image_utils.h"
#include <cstdlib>
#include <ctime>

int main()
{
	TabletTestApp app;
	while (!app.ended())
	{
		app.update();
	}
	return 0;
}


static uint8_t rand_byte()
{
	return (std::rand() & 0xff);
}

static double rand_num()
{
	return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
}

static void randomize_tablet(TabletTestModel::TabletItem& tablet, double prob)
{
	for (auto& glyph : tablet.glyphs)
	{
		if (rand_num() <= prob)
		{
			glyph.color1.r = rand_byte();
			glyph.color1.g = rand_byte();
			glyph.color1.b = rand_byte();
			glyph.color1.a = 255;
			glyph.color2.r = rand_byte();
			glyph.color2.g = rand_byte();
			glyph.color2.b = rand_byte();
			glyph.color2.a = 255;
			glyph.code = rand_byte();
		}
	}
}

TabletTestApp::TabletTestApp()
{
	// set a seed
	std::srand(std::time(nullptr));

	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = "Tablet Test";
	window = Window::create(params);

	renderer = std::make_unique<TabletTestRenderer>(*window);


	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	store.tablet_shader = Shader::create(desc);

	desc.vs_path = "../../data/shaders/basic_textured_vs.gls";
	desc.fs_path = "../../data/shaders/basic_textured_fs.gls";
	store.tablet_screen_shader = Shader::create(desc);

	// TODO: the atlas texture should probably use nearest filter 
	// since we are drawing into pixel perfect render buffer
	auto tex_desc = load_texture("../../data/fonts/cp437_20x20.png");
	store.atlas_texture = Texture::create(tex_desc);

	int width = 120;
	int height = 80;

	const auto id = store.tablet_store.add_tablet(width, height, store.atlas_texture.id(), store.tablet_shader, store.tablet_screen_shader);

	const auto fixed_tablet_id = store.tablet_store.add_tablet(16, 16, store.atlas_texture.id(), store.tablet_shader, store.tablet_screen_shader);

	// create a item
	model.cam_pose = make_lookat(Vec3{0, 0, -60}, Vec3{0, 0, 0}, Vec3{0, 1, 0});
	model.tablets = 
	{ 
		{ id, Pose{}, width, height, {}, std::vector<GlyphData>(width * height) },
		{ fixed_tablet_id, Pose{ {0.f, 0.f, -1.f} }, 16, 16, {}, std::vector<GlyphData>(16 * 16) },
	};

	randomize_tablet(model.tablets[0], 1.0);

	for (size_t i = 0; i < model.tablets[1].glyphs.size(); i++)
	{
		auto& glyph = model.tablets[1].glyphs[i];		
		glyph.code = i;
		glyph.color1.r = (255 - i);
		glyph.color2.g = i;
		glyph.color1.g = glyph.color1.b = glyph.color2.r = glyph.color2.b = 0;
		glyph.color1.a = 255;
		glyph.color2.a = 255;
	}

	start_time = std::chrono::system_clock::now();
}

void TabletTestApp::update()
{
	window->poll_events();

	// randomize all glyphs
	randomize_tablet(model.tablets[0], 0.01);

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

