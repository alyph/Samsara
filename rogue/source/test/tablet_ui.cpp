
#include "tablet_ui.h"
#include "engine/input.h"
#include "engine/viewport.h"


int main()
{
	TabletUIApp app;
	while (!app.ended())
	{
		app.update();
	}
	return 0;
}

TabletUIApp::TabletUIApp()
{
	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = "Tablet UI";
	window = Window::create(params);

	presenter.set_present_object(this);
}

void TabletUIApp::update()
{
	auto events = window->poll_events();
	presenter.process_control(events);

	// do this if we update model per frame
	// presenter.process_view();

	// push render batch
	const double dt = 1.0 / 60.0;
	presenter.step_frame(dt);

	// if (!events.empty())
	// {
	// 	printf("%zu events:", events.size());
	// 	for (const auto& event : events)
	// 	{
	// 		if (event.type == InputEventType::key_down)
	// 		{
	// 			printf(" key_down(%d)", event.key);
	// 		}
	// 		else if (event.type == InputEventType::key_up)
	// 		{
	// 			printf(" key_up(%d)", event.key);
	// 		}
	// 		else if (event.type == InputEventType::mouse_down)
	// 		{
	// 			printf(" mouse_down(%d)", event.button);
	// 		}
	// 		else if (event.type == InputEventType::mouse_up)
	// 		{
	// 			printf(" mouse_up(%d)", event.button);
	// 		}
	// 		else if (event.type == InputEventType::mouse_move)
	// 		{
	// 			printf(" mouse_move(%f,%f)", event.x, event.y);
	// 		}
	// 	}
	// 	printf("\n");
	// }

	// if (false);

	// present(*this);

	// present
	window->present();
}

void TabletUIApp::present(const Context ctx)
{
	using namespace elem;

	viewport(_ctx); _tag("viewport");_children 
	{		
		tablet(_ctx); _tag("tablet wide"); _children
		{
			std::string str = "Dream Park is a futuristic amusement park using holograms and other advanced technologies to entertain customers, including live-action role-players. Dream Park, The Barsoom Project and The California Voodoo Game follow security chief Alex Griffin as he attempts to solve various mysteries set in the park. The other stories in this series have only a peripheral connection. Saturn's Race is a prequel to Achilles' Choice; both involve young adults technologically \"upgrading\" their bodies in an effort to join the world's ruling elite."
			text(_ctx, str);
		}
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
	return window->should_close();
}


