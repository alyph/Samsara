#include "game.h"
#include "engine/window.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/image_utils.h"

Game::Game()
{
	// TODO: move to asset manager or something
	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	tablet_shader = create_shader(desc);

	desc.vs_path = "../../data/shaders/basic_textured_vs.gls";
	desc.fs_path = "../../data/shaders/basic_textured_fs.gls";
	tablet_screen_shader = create_shader(desc);

	auto tex_desc = load_texture("../../data/fonts/cp437_20x20.png");
	atlas_texture = Texture::create(tex_desc);
}


void Game::update()
{
}

void Game::present(const Context& ctx)
{
	using namespace elem;

	auto window = engine().window;

	const float aspect = window->aspect();

	const int tablet_width = 160;
	const int tablet_height = 80;
	const Vec2 tablet_size = calc_tablet_size(tablet_width, tablet_height, atlas_texture.id());

	const float vp_width = (float)tablet_width;
	const float vp_height = vp_width / aspect;

	// TODO: use othographic projection
	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);
	vp.pose.pos.y = ((float)tablet_size.y - vp_height) / 2;

	viewport(_ctx);
	_attr(attrs::width, window->width());
	_attr(attrs::height, window->height());
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		tablet(_ctx);

		
		_attr(attrs::transform, Mat44::identity());
		_attr(attrs::width, tablet_width);
		_attr(attrs::height, tablet_height);
		_attr(attrs::texture, atlas_texture.id());
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			node(_ctx);
			_attr(attrs::height, 2);

			node(_ctx);
			String str = "Dream Park is a futuristic amusement park using holograms and other advanced technologies to entertain customers, including live-action role-players. Dream Park, The Barsoom Project and The California Voodoo Game follow security chief Alex Griffin as he attempts to solve various mysteries set in the park. The other stories in this series have only a peripheral connection. Saturn's Race is a prequel to Achilles' Choice; both involve young adults technologically \"upgrading\" their bodies in an effort to join the world's ruling elite.";
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, (Color{0.8f, 0.1f, 0.5f, 1.f}));

			node(_ctx);
			str = "---------------------------------------------------------------------";
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, (Color{0.8f, 0.8f, 0.8f, 1.f}));


			node(_ctx);
			str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+-*/={}'\"?<>|\\[]:,.";
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, (Color{0.2f, 0.9f, 0.7f, 1.f}));
			_attr(attrs::width, 50);


			node(_ctx);
			str = format_str("[ click me! (%d) ]", click_count);
			_attr(attrs::text, str);
			
			if (_down)
			{
				_attr(attrs::foreground_color, (Color{1.f, 1.f, 1.f, 1.f}));
				_attr(attrs::background_color, (Color{1.f, 0.5f, 0.5f, 1.f}));
			}
			else if (_hover)
			{
				_attr(attrs::foreground_color, (Color{1.f, 1.f, 1.f, 1.f}));
				_attr(attrs::background_color, (Color{0.8f, 0.1f, 0.1f, 1.f}));
			}
			else
			{
				_attr(attrs::foreground_color, (Color{0.8f, 0.1f, 0.1f, 1.f}));
				_attr(attrs::background_color, (Color{1.f, 1.f, 1.f, 1.f}));
			}

			if (_clicked)
			{
				click_count++;
			}
		}
	}
}

bool Game::ended()
{
	return engine().window->should_close();
}
