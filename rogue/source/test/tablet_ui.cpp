
#include "tablet_ui.h"
#include "engine/app.h"
#include "engine/viewport.h"
#include "engine/math_utils.h"
#include "engine/tablet.h"


int main()
{
	return run_app<TabletUIApp>();
}

TabletUIApp::TabletUIApp()
{
}

void TabletUIApp::update()
{
}

void TabletUIApp::present(const Context& ctx)
{
	using namespace elem;

	auto window = engine().window;

	// TODO: use othographic projection
	Viewpoint vp;
	vp.projection = make_perspective(to_rad(60.f), window->aspect(), 0.1f, 100.f);
	vp.pose = make_lookat(Vec3{0, 0, -60}, Vec3{0, 0, 0}, Vec3{0, 1, 0});

	viewport(_ctx);
	_attr(attrs::width, static_cast<double>(window->width()));
	_attr(attrs::height, static_cast<double>(window->height()));
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		tablet(_ctx);

		TempString str = "Dream Park is a futuristic amusement park using holograms and other advanced technologies to entertain customers, including live-action role-players. Dream Park, The Barsoom Project and The California Voodoo Game follow security chief Alex Griffin as he attempts to solve various mysteries set in the park. The other stories in this series have only a peripheral connection. Saturn's Race is a prequel to Achilles' Choice; both involve young adults technologically \"upgrading\" their bodies in an effort to join the world's ruling elite.";

		_attr(attrs::transform, to_mat44(tablet_model.pose));
		_attr(attrs::text, str);
		_attr(attrs::width, static_cast<double>(tablet_model.width));
		_attr(attrs::height, static_cast<double>(tablet_model.height));			
		_attr(attrs::texture, store.atlas_texture.id());
		_attr(attrs::shader, store.tablet_shader);
		_attr(attrs::quad_shader, store.tablet_screen_shader);
	}

	// auto vp = []() { static Id my_root_id = presenter.new_id(); return viewport(presenter, my_root_id); }();

	// _elem(viewport); _group
	// {
	// 	root_tab = _elem(tablet);
		
	// 	CLICK {  }
	// 	HOVER {  }

	// 	_group
	// 	{
	// 		_elem(pane); _group
	// 		{
	// 			_elem(pane); _group
	// 			{
	// 				_elem(text);
	// 				_elem(border);
	// 			}
	// 			_elem(button);
	// 		}	
	// 	}
	// }
}

bool TabletUIApp::ended()
{
	return engine().window->should_close();
}


