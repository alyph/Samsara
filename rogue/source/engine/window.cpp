
#include "window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

std::unique_ptr<Window> Window::create(const WindowCreationParams& params)
{	
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return nullptr;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	auto glfw = glfwCreateWindow(params.width, params.height, params.title.c_str(), NULL, NULL);
	if (!glfw)
	{
		fprintf( stderr, "Failed to open GLFW window. Make sure the input params are correct.\n" );
		getchar();
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(glfw);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return nullptr;
	}

	// vsync
	glfwSwapInterval(1);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(glfw, GLFW_STICKY_KEYS, GL_TRUE);

	glViewport(0, 0, params.width, params.height);

	auto window = std::make_unique<Window>();
	window->glfw = glfw;
	glfwSetWindowUserPointer(glfw, window.get());

	glfwSetKeyCallback(glfw, key_callback);
	glfwSetCursorPosCallback(glfw, mouse_position_callback);
	glfwSetMouseButtonCallback(glfw, mouse_button_callback);

	return window;
}

Window::~Window()
{
	glfwDestroyWindow(glfw);
	glfwTerminate();
}

void Window::present()
{
	glfwSwapBuffers(glfw);
}

std::vector<InputEvent> Window::poll_events()
{
	glfwPollEvents();
	std::vector<InputEvent> new_events = input_events;
	input_events.clear();
	return new_events;
}

bool Window::should_close() const
{
	return glfwWindowShouldClose(glfw);
}

float Window::aspect() const
{
	int width{}, height{};
	glfwGetWindowSize(glfw, &width, &height);
	return (static_cast<float>(width) / height);
}

int Window::width() const
{
	int width{}, height{};
	glfwGetWindowSize(glfw, &width, &height);
	return width;
}

int Window::height() const
{
	int width{}, height{};
	glfwGetWindowSize(glfw, &width, &height);
	return height;
}

void Window::key_callback(GLFWwindow* glfw, int key, int scancode, int action, int mods)
{
	auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw));
	if (window)
	{
		InputEvent event;
		if (action == GLFW_PRESS)
		{
			event.type = InputEventType::key_down;
		}
		else if (action == GLFW_RELEASE)
		{
			event.type = InputEventType::key_up;
		}
		else
		{
			return;
		}
		event.key = key;
		event.mods = mods;
		window->input_events.push_back(event);
	}
}

void Window::mouse_position_callback(GLFWwindow* glfw, double xpos, double ypos)
{
	auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw));
	if (window)
	{
		InputEvent event;
		event.type = InputEventType::mouse_move;
		event.x = xpos;
		event.y = ypos;
		window->input_events.push_back(event);
	}
}

void Window::mouse_button_callback(GLFWwindow* glfw, int button, int action, int mods)
{
	auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw));
	if (window)
	{
		InputEvent event;
		if (action == GLFW_PRESS)
		{
			event.type = InputEventType::mouse_down;
		}
		else if (action == GLFW_RELEASE)
		{
			event.type = InputEventType::mouse_up;
		}
		else
		{
			return;
		}
		event.button = button;
		event.mods = mods;
		window->input_events.push_back(event);
	}
}

