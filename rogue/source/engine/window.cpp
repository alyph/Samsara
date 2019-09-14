
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

	// TODO: add an intiial move move event in case mouse doesn't move for the first couple of frames
	// TODO: may also consider add initial mouse, key down events but to do that we handle that as part of the poll_events()

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

void Window::set_title(const String& title)
{
	glfwSetWindowTitle(glfw, title.c_str());
}

// make sure our mapping to glfw keys is consistent
static_assert((int)Keys::space == GLFW_KEY_SPACE);
static_assert((int)Keys::apostrophe == GLFW_KEY_APOSTROPHE);
static_assert((int)Keys::comma == GLFW_KEY_COMMA);
static_assert((int)Keys::minus == GLFW_KEY_MINUS);
static_assert((int)Keys::period == GLFW_KEY_PERIOD);
static_assert((int)Keys::slash == GLFW_KEY_SLASH);
static_assert((int)Keys::num0 == GLFW_KEY_0);
static_assert((int)Keys::num9 == GLFW_KEY_9);
static_assert((int)Keys::semicolon == GLFW_KEY_SEMICOLON);
static_assert((int)Keys::equal == GLFW_KEY_EQUAL);
static_assert((int)Keys::a == GLFW_KEY_A);
static_assert((int)Keys::m == GLFW_KEY_M);
static_assert((int)Keys::z == GLFW_KEY_Z);
static_assert((int)Keys::left_bracket == GLFW_KEY_LEFT_BRACKET);
static_assert((int)Keys::backslash == GLFW_KEY_BACKSLASH);
static_assert((int)Keys::right_bracket == GLFW_KEY_RIGHT_BRACKET);
static_assert((int)Keys::grave_accent == GLFW_KEY_GRAVE_ACCENT);
static_assert((int)Keys::escape == GLFW_KEY_ESCAPE - 128);
static_assert((int)Keys::enter == GLFW_KEY_ENTER - 128);
static_assert((int)Keys::tab == GLFW_KEY_TAB - 128);
static_assert((int)Keys::backspace == GLFW_KEY_BACKSPACE - 128);
static_assert((int)Keys::insert == GLFW_KEY_INSERT - 128);
static_assert((int)Keys::del == GLFW_KEY_DELETE - 128);
static_assert((int)Keys::right == GLFW_KEY_RIGHT - 128);
static_assert((int)Keys::left == GLFW_KEY_LEFT - 128);
static_assert((int)Keys::down == GLFW_KEY_DOWN - 128);
static_assert((int)Keys::up == GLFW_KEY_UP - 128);
static_assert((int)Keys::page_up == GLFW_KEY_PAGE_UP - 128);
static_assert((int)Keys::page_down == GLFW_KEY_PAGE_DOWN - 128);
static_assert((int)Keys::home == GLFW_KEY_HOME - 128);
static_assert((int)Keys::end == GLFW_KEY_END - 128);
static_assert((int)Keys::caps_lock == GLFW_KEY_CAPS_LOCK - 128);
static_assert((int)Keys::scroll_lock == GLFW_KEY_SCROLL_LOCK - 128);
static_assert((int)Keys::num_lock == GLFW_KEY_NUM_LOCK - 128);
static_assert((int)Keys::print_screen == GLFW_KEY_PRINT_SCREEN - 128);
static_assert((int)Keys::pause == GLFW_KEY_PAUSE - 128);
static_assert((int)Keys::f1 == GLFW_KEY_F1 - 128);
static_assert((int)Keys::f10 == GLFW_KEY_F10 - 128);
static_assert((int)Keys::f20 == GLFW_KEY_F20 - 128);
static_assert((int)Keys::f25 == GLFW_KEY_F25 - 128);
static_assert((int)Keys::kp_0 == GLFW_KEY_KP_0 - 128);
static_assert((int)Keys::kp_decimal == GLFW_KEY_KP_DECIMAL - 128);
static_assert((int)Keys::kp_equal == GLFW_KEY_KP_EQUAL - 128);
static_assert((int)Keys::left_shift == GLFW_KEY_LEFT_SHIFT - 128);
static_assert((int)Keys::left_control == GLFW_KEY_LEFT_CONTROL - 128);
static_assert((int)Keys::left_alt == GLFW_KEY_LEFT_ALT - 128);
static_assert((int)Keys::left_super == GLFW_KEY_LEFT_SUPER - 128);
static_assert((int)Keys::right_shift == GLFW_KEY_RIGHT_SHIFT - 128);
static_assert((int)Keys::right_control == GLFW_KEY_RIGHT_CONTROL - 128);
static_assert((int)Keys::right_alt == GLFW_KEY_RIGHT_ALT - 128);
static_assert((int)Keys::right_super == GLFW_KEY_RIGHT_SUPER - 128);
static_assert((int)Keys::menu == GLFW_KEY_MENU - 128);
static_assert((int)Keys::max == GLFW_KEY_LAST - 128 + 1);

// match glfw mouse buttons
static_assert((int)MouseButtons::button1 == GLFW_MOUSE_BUTTON_1);
static_assert((int)MouseButtons::button2 == GLFW_MOUSE_BUTTON_2);
static_assert((int)MouseButtons::button3 == GLFW_MOUSE_BUTTON_3);
static_assert((int)MouseButtons::button4 == GLFW_MOUSE_BUTTON_4);
static_assert((int)MouseButtons::button8 == GLFW_MOUSE_BUTTON_8);
static_assert((int)MouseButtons::max == GLFW_MOUSE_BUTTON_LAST + 1);
static_assert((int)MouseButtons::left == GLFW_MOUSE_BUTTON_LEFT);
static_assert((int)MouseButtons::right == GLFW_MOUSE_BUTTON_RIGHT);
static_assert((int)MouseButtons::middle == GLFW_MOUSE_BUTTON_MIDDLE);


void Window::key_callback(GLFWwindow* glfw, int key, int scancode, int action, int mods)
{
	auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw));
	if (window)
	{
		InputEvent event;
		event.type = InputEventType::key;
		if (action == GLFW_PRESS)
		{
			event.down = true;
		}
		else if (action == GLFW_RELEASE)
		{
			event.down = false;
		}
		else
		{
			return;
		}

		if (key >= 256 && key <= GLFW_KEY_LAST)
		{
			event.key = (Keys)(key - 128);
		}
		else if (key < 128)
		{
			event.key = (Keys)key;
		}
		// TODO: any keys between 128 and 256 are thrown away, insert those somewhere if we need them
		// currently just the world keys which we may not need

		// TODO: do we need mods?
		// event.mods = mods;
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
		event.type = InputEventType::mouse_button;
		if (action == GLFW_PRESS)
		{
			event.down = true;
		}
		else if (action == GLFW_RELEASE)
		{
			event.down = false;
		}
		else
		{
			return;
		}
		event.button = (MouseButtons)button;
		// TODO: do we need mods?
		// event.mods = mods;
		window->input_events.push_back(event);
	}
}

