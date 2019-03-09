#pragma once

#include "window.h"
#include "input.h"
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
	params.title = "Tablet Test";
	auto window = Window::create(params);
	engine().window = window.get();

	AppT app();
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