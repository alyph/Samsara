#pragma once
#include "input.h"
#include "string.h"
#include <memory>
#include <string>
#include <vector>

struct InputEvent;

struct WindowCreationParams
{
	String title;
	int width{};
	int height{};
};

class Window
{
public:
	void *wnd{}, *dc{}, *rc{};

	~Window();

	static std::unique_ptr<Window> create(const WindowCreationParams& params);

	void present();
	std::vector<InputEvent> poll_events();
	bool should_close() const;
	float aspect() const;
	int width() const;
	int height() const;
	void set_title(const String& title);

private:
	bool request_close{};
};


