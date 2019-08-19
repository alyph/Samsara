#pragma once

#include "window.h"
#include "input.h"
#include "os.h"
#include "easy/profiler.h"
#include <chrono>

template<class AppT>
int run_app()
{
	profiler::startListen();
	scoped_engine_init();

	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = get_executable_name();
	StringStore title = params.title;
	auto window = Window::create(params);
	engine().window = window.get();

	AppT app;
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

		// clean up allocators (deallocate memory when needed)
		engine().allocators.regular_cleanup();

		// TODO: reuse this code
		frame_count++;
		auto now = std::chrono::system_clock::now();
		if ((now - start_time) >= std::chrono::duration<double>(1.0))
		{
			// TODO: use String's function for concatenating or formating strings
			std::string title_and_fps = title.c_str();
			title_and_fps += "  fps: ";
			title_and_fps += std::to_string(std::lround(frame_count / (std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time).count())));
			window->set_title(String{title_and_fps.data(), title_and_fps.length()});
			start_time = now;
			frame_count = 0;
		}
	}
	return 0;
}
