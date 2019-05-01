#pragma once
#include "input.h"
#include "string.h"
#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;
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
	static void key_callback(GLFWwindow* glfw, int key, int scancode, int action, int mods);
	static void mouse_position_callback(GLFWwindow* glfw, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* glfw, int button, int action, int mods);

	GLFWwindow* glfw{};
	std::vector<InputEvent> input_events;
};


