
#include "tablet_test.h"
#include "engine/renderer.h"
#include "engine/math_utils.h"
#include "engine/image_utils.h"
#include "engine/viewport.h"
#include "easy/profiler.h"
#include <cstdlib>
#include <ctime>

int main()
{
	profiler::startListen();

	scoped_engine_init();

	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = "Tablet Test";
	auto window = Window::create(params);

	TabletTestApp app(window.get());
	Presenter presenter;
	presenter.set_present_object(&app);

	int frame_count{};
	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

	// TabletTestApp app;
	// set_present_func(present_func, std::ref(app.engine), std::ref(app.model));
	while (!app.ended())
	{
		presenter.process_control(window->poll_events());
		
		app.update();

		// render stuff out
		//renderer->render(store, model);

		const double dt = 1.0 / 60.0;
		presenter.step_frame(dt);

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
	EASY_FUNCTION();

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

TabletTestApp::TabletTestApp(Window* window):
	window(window)
{
	// set a seed
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// WindowCreationParams params;
	// params.width = 1024;
	// params.height = 768;
	// params.title = "Tablet Test";
	// window = Window::create(params);

	// renderer = std::make_unique<TabletTestRenderer>(*window);
	// presenter.set_present_object(this);


	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	store.tablet_shader = create_shader(desc);

	desc.vs_path = "../../data/shaders/basic_textured_vs.gls";
	desc.fs_path = "../../data/shaders/basic_textured_fs.gls";
	store.tablet_screen_shader = create_shader(desc);

	// TODO: the atlas texture should probably use nearest filter 
	// since we are drawing into pixel perfect render buffer
	auto tex_desc = load_texture("../../data/fonts/cp437_20x20.png");
	store.atlas_texture = Texture::create(tex_desc);

	int width = 120;
	int height = 80;

	// const auto id = add_tablet(width, height, store.atlas_texture.id(), store.tablet_shader, store.tablet_screen_shader);

	// const auto fixed_tablet_id = add_tablet(16, 16, store.atlas_texture.id(), store.tablet_shader, store.tablet_screen_shader);

	// create a item
	model.cam_pose = make_lookat(Vec3{0, 0, -60}, Vec3{0, 0, 0}, Vec3{0, 1, 0});
	model.tablets = 
	{ 
		{ Pose{}, width, height, {}, alloc_simple_array<GlyphData>(width * height) },
		{ Pose{ {0.f, 0.f, -1.f} }, 16, 16, {}, alloc_simple_array<GlyphData>(16 * 16) },
	};

	randomize_tablet(model.tablets[0], 1.0);

	for (size_t i = 0; i < model.tablets[1].glyphs.size(); i++)
	{
		auto& glyph = model.tablets[1].glyphs[i];		
		glyph.code = static_cast<decltype(glyph.code)>(i);
		glyph.color1.r = static_cast<decltype(glyph.color1.r)>(255 - i);
		glyph.color2.g = static_cast<decltype(glyph.color2.g)>(i);
		glyph.color1.g = glyph.color1.b = glyph.color2.r = glyph.color2.b = 0;
		glyph.color1.a = 255;
		glyph.color2.a = 255;
	}

	// start_time = std::chrono::system_clock::now();
}

void TabletTestApp::update()
{
	EASY_FUNCTION();

	// presenter.process_control(window->poll_events());

	// randomize all glyphs
	randomize_tablet(model.tablets[0], 0.01);

	// render stuff out
	//renderer->render(store, model);

	// const double dt = 1.0 / 60.0;
	// presenter.step_frame(dt);

	// // present
	// window->present();

	// TODO: reuse this code
	// frame_count++;
	// auto now = std::chrono::system_clock::now();
	// if ((now - start_time) >= std::chrono::duration<double>(1.0))
	// {
	// 	printf("-- fps: %f\n", frame_count / (std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time).count()));
	// 	start_time = now;
	// 	frame_count = 0;
	// }
}

bool TabletTestApp::ended()
{
	return window->should_close();
}


void TabletTestApp::present(const Context& ctx)
{
	using namespace elem;

	Viewpoint vp;
	vp.projection = make_perspective(to_rad(60.f), window->aspect(), 0.1f, 100.f);
	vp.pose = model.cam_pose;	

	viewport(_ctx);
	_attr(attrs::width, static_cast<double>(window->width()));
	_attr(attrs::height, static_cast<double>(window->height()));
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		for (size_t i = 0; i < model.tablets.size(); i++)
		{
			auto& tablet_model = model.tablets[i];
			tablet(_ctx_id(i));
			_attr(attrs::transform, to_mat44(tablet_model.pose));
			_attr(attrs::glyphs, tablet_model.glyphs);
			_attr(attrs::width, static_cast<double>(tablet_model.width));
			_attr(attrs::height, static_cast<double>(tablet_model.height));			
			_attr(attrs::texture, store.atlas_texture.id());
			_attr(attrs::shader, store.tablet_shader);
			_attr(attrs::quad_shader, store.tablet_screen_shader);
		}
	}
}


