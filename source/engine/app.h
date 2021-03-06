#pragma once

#include "window.h"
#include "input.h"
#include "os.h"
#include "time.h"
#include "easy/profiler.h"
#include <chrono>

struct AppConfig
{
	int window_width = 1024;
	int window_height = 768;
};

const int64_t compute_elapsed_nano_seconds(std::chrono::high_resolution_clock::duration elapsed_time)
{
	return std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>(elapsed_time).count();
}

template<class AppT>
int run_app(const AppConfig& config = {})
{
	profiler::startListen();
	scoped_engine_init();

	const auto app_start_time = std::chrono::high_resolution_clock::now();

	WindowCreationParams params;
	params.width = config.window_width;
	params.height = config.window_height;
	params.title.store(get_executable_name(), Allocator::app);
	auto window = Window::create(params);
	if (!window) {
		fprintf(stderr, "failed to create app window\n");
		return 1;
	}

	engine().window = window.get();

	// const auto app_start = std::chrono::system_clock::now();
	AppT app;
	// const auto app_end = std::chrono::system_clock::now();
	// printf("startup time: %f\n", std::chrono::duration_cast<std::chrono::duration<double>>(app_end - app_start).count());
	Presenter presenter;
	presenter.set_present_object(&app);

	int frame_count{};
	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

	std::chrono::high_resolution_clock::time_point prev_update_time = std::chrono::high_resolution_clock::now();

	while (!app.ended())
	{
		presenter.process_control(window->poll_events());
		
		auto current_update_time = std::chrono::high_resolution_clock::now();
		Time time
		{
			.current_time_ns = compute_elapsed_nano_seconds(current_update_time - app_start_time),
			.delta_time_ns = compute_elapsed_nano_seconds(current_update_time - prev_update_time),
		};
		prev_update_time = current_update_time;

		app.update(time);
		presenter.step_frame(time);

		// present
		window->present();

		// clean up allocators (deallocate memory when needed)
		engine().allocators.regular_cleanup();

		frame_count++;
		auto now = std::chrono::system_clock::now();
		if ((now - start_time) >= std::chrono::duration<double>(1.0))
		{
			const auto fps = std::lround(frame_count / (std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time).count()));
			window->set_title(format_str("%s fps: %d", params.title, fps));
			start_time = now;
			frame_count = 0;
		}
	}
	app.shutdown();
	return 0;
}
