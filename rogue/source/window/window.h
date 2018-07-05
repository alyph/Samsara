#pragma once
#include <memory>
#include <string>

struct GLFWwindow;

struct WindowCreationParams
{
	std::string title;
	int width{};
	int height{};
};

class Window
{
public:
	~Window();

	static std::unique_ptr<Window> create(const WindowCreationParams& params);

	void present();
	void poll_events();
	bool should_close() const;
	float aspect() const;

private:
	GLFWwindow* glfw{};
};


