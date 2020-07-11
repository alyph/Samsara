
#include "window.h"
#include <GL/glew.h>
#include <GL/wglew.h>
// #include <GLFW/glfw3.h>
// #include <GL/gl3w.h>
// #include <GL/glext.h>
// #include <GL/wglext.h>
#include <windows.h>
#include <windowsx.h>
#include <chrono>

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			// TODO: this skips alt+F4, should probably enable that?
			// also what handles alt+enter?
			break;
		case WM_MENUCHAR:
			return MNC_CLOSE << 16;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;		// message handled
}


std::unique_ptr<Window> Window::create(const WindowCreationParams& params)
{
	// auto begin = std::chrono::high_resolution_clock::now();

	const auto inst = GetModuleHandleW(NULL);

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = window_proc;
	wcex.hInstance = inst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = "MainWindow";

	LPTSTR window_class = MAKEINTATOM(RegisterClassEx(&wcex));
	if (window_class == 0) 
	{
		fprintf(stderr, "registerClass() failed.");
		return nullptr;
	}

	// auto reg_wnd_cls = std::chrono::high_resolution_clock::now();

	// create temporary window
	HWND fake_wnd = CreateWindow(
		window_class, "Fake Window",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0,						// position x, y
		1, 1,						// width, height
		NULL, NULL,					// parent window, menu
		inst, NULL);				// instance, param

	HDC fake_dc = GetDC(fake_wnd);	// Device Context

	PIXELFORMATDESCRIPTOR fake_pfd;
	ZeroMemory(&fake_pfd, sizeof(fake_pfd));
	fake_pfd.nSize = sizeof(fake_pfd);
	fake_pfd.nVersion = 1;
	fake_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fake_pfd.iPixelType = PFD_TYPE_RGBA;
	fake_pfd.cColorBits = 32;
	fake_pfd.cAlphaBits = 8;
	fake_pfd.cDepthBits = 24;

	const int fake_pfd_id = ChoosePixelFormat(fake_dc, &fake_pfd);
	if (fake_pfd_id == 0) 
	{
		fprintf(stderr, "ChoosePixelFormat() failed.");
		return nullptr;
	}
	// auto first_choose_pixel = std::chrono::high_resolution_clock::now();

	if (SetPixelFormat(fake_dc, fake_pfd_id, &fake_pfd) == false) {
		fprintf(stderr, "SetPixelFormat() failed.");
		return nullptr;
	}
	// auto first_set_pixel = std::chrono::high_resolution_clock::now();

	HGLRC fake_rc = wglCreateContext(fake_dc);	// Rendering Contex
	if (fake_rc == 0) 
	{
		fprintf(stderr, "wglCreateContext() failed.");
		return nullptr;
	}

	if (wglMakeCurrent(fake_dc, fake_rc) == false) 
	{
		fprintf(stderr, "wglMakeCurrent() failed.");
		return nullptr;
	}

	// auto before_gl3w = std::chrono::high_resolution_clock::now();


	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GL3W\n");
		return nullptr;
	}

	RECT rect = { 0, 0, params.width, params.height };
	DWORD style = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_THICKFRAME;
	AdjustWindowRectEx(&rect, style, FALSE, 0);

	const auto wnd_width = rect.right - rect.left;
    const auto wnd_height = rect.bottom - rect.top;

	RECT display_size;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &display_size, 0);	// system taskbar and application desktop toolbars not included
	const auto wnd_x = (display_size.right - wnd_width) / 2;
	const auto wnd_y = (display_size.bottom - wnd_height) / 2;

	HWND wnd = CreateWindow(
		window_class, params.title.c_str(),	// class name, window name
		style,							// styles
		wnd_x, wnd_y,		// posx, posy. If x is set to CW_USEDEFAULT y is ignored
		wnd_width, wnd_height,	// width, height
		NULL, NULL,						// parent window, menu
		inst, NULL);				// instance, param

	HDC dc = GetDC(wnd);

	// auto created_real_win = std::chrono::high_resolution_clock::now();

	const int pixel_attrs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 4,
		0
	};

	int pf_id; UINT num_formats;
	const bool status = wglChoosePixelFormatARB(dc, pixel_attrs, NULL, 1, &pf_id, &num_formats);

	if (status == false || num_formats == 0) {
		fprintf(stderr, "wglChoosePixelFormatARB() failed.");
		return nullptr;
	}

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(dc, pf_id, sizeof(pfd), &pfd);
	SetPixelFormat(dc, pf_id, &pfd);

	// auto set_pixel_form = std::chrono::high_resolution_clock::now();

	const int major_min = 4, minor_min = 5;
	const int context_attrs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
//		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0
	};

	HGLRC rc = wglCreateContextAttribsARB(dc, 0, context_attrs);
	if (rc == NULL) {
		fprintf(stderr, "wglCreateContextAttribsARB() failed.");
		return nullptr;
	}

	// delete temporary context and window
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fake_rc);
	ReleaseDC(fake_wnd, fake_dc);
	DestroyWindow(fake_wnd);
	if (!wglMakeCurrent(dc, rc)) {
		fprintf(stderr, "wglMakeCurrent() failed.");
		return nullptr;
	}

	// vsync on
	wglSwapIntervalEXT(1);

	ShowWindow(wnd, SW_SHOW);

	static_assert(sizeof(HWND) == sizeof(void*));
	static_assert(sizeof(HDC) == sizeof(void*));
	static_assert(sizeof(HGLRC) == sizeof(void*));

	auto window = std::make_unique<Window>();
	window->wnd = reinterpret_cast<void*>(wnd);
	window->dc = reinterpret_cast<void*>(dc);
	window->rc = reinterpret_cast<void*>(rc);


	// TODO: add an intiial move move event in case mouse doesn't move for the first couple of frames
	// TODO: may also consider add initial mouse, key down events but to do that we handle that as part of the poll_events()

	// auto end = std::chrono::high_resolution_clock::now();

	// printf("reg wnd class:    %llu\n", (reg_wnd_cls - begin).count() / 1000);
	// printf("create fake:      %llu\n", (created_fake - reg_wnd_cls).count() / 1000);
	// printf("choose pixel:     %llu\n", (first_choose_pixel - created_fake).count() / 1000);
	// printf("set pixel:        %llu\n", (first_set_pixel - first_choose_pixel).count() / 1000);
	// printf("temp rc:          %llu\n", (before_gl3w - first_set_pixel).count() / 1000);
	// printf("glew init:        %llu\n", (after_gl3w - before_gl3w).count() / 1000);
	// printf("create real:      %llu\n", (created_real_win - after_gl3w).count() / 1000);
	// printf("set pixel format: %llu\n", (set_pixel_form - created_real_win).count() / 1000);
	// printf("gl context:       %llu\n", (end - set_pixel_form).count() / 1000);

	return window;
}

static inline HWND get_handle(const Window& window)
{
	return reinterpret_cast<HWND>(window.wnd);
}

Window::~Window()
{
	wglMakeCurrent(NULL, NULL);
	if (rc) {
		wglDeleteContext(reinterpret_cast<HGLRC>(rc));
	}
	if (dc) {
		ReleaseDC(get_handle(*this), reinterpret_cast<HDC>(dc));
	}
	if (wnd) {
		DestroyWindow(get_handle(*this));
	}
}

void Window::present()
{
	SwapBuffers(reinterpret_cast<HDC>(dc));
}

static void queue_input_event(std::vector<InputEvent>& events, UINT message, WPARAM wparam, LPARAM lparam)
{
	// verify consistency with windows virtual key codes
	static_assert((int)Keys::space == VK_SPACE);
	static_assert((int)Keys::quote == VK_OEM_7);
	static_assert((int)Keys::comma == VK_OEM_COMMA);
	static_assert((int)Keys::minus == VK_OEM_MINUS);
	static_assert((int)Keys::period == VK_OEM_PERIOD);
	static_assert((int)Keys::slash == VK_OEM_2);
	static_assert((int)Keys::num0 == '0');
	static_assert((int)Keys::num9 == '9');
	static_assert((int)Keys::semicolon == VK_OEM_1);
	static_assert((int)Keys::equal == VK_OEM_PLUS);
	static_assert((int)Keys::a == 'A');
	static_assert((int)Keys::m == 'M');
	static_assert((int)Keys::z == 'Z');
	static_assert((int)Keys::left_bracket == VK_OEM_4);
	static_assert((int)Keys::backslash == VK_OEM_5);
	static_assert((int)Keys::right_bracket == VK_OEM_6);
	static_assert((int)Keys::grave_accent == VK_OEM_3);
	static_assert((int)Keys::escape == VK_ESCAPE);
	static_assert((int)Keys::enter == VK_RETURN);
	static_assert((int)Keys::tab == VK_TAB);
	static_assert((int)Keys::backspace == VK_BACK);
	static_assert((int)Keys::insert == VK_INSERT);
	static_assert((int)Keys::del == VK_DELETE);
	static_assert((int)Keys::right == VK_RIGHT);
	static_assert((int)Keys::left == VK_LEFT);
	static_assert((int)Keys::down == VK_DOWN);
	static_assert((int)Keys::up == VK_UP);
	static_assert((int)Keys::page_up == VK_PRIOR);
	static_assert((int)Keys::page_down == VK_NEXT);
	static_assert((int)Keys::home == VK_HOME);
	static_assert((int)Keys::end == VK_END);
	static_assert((int)Keys::caps_lock == VK_CAPITAL);
	static_assert((int)Keys::scroll_lock == VK_SCROLL);
	static_assert((int)Keys::num_lock == VK_NUMLOCK);
	static_assert((int)Keys::print_screen == VK_SNAPSHOT);
	static_assert((int)Keys::pause == VK_PAUSE);
	static_assert((int)Keys::f1 == VK_F1);
	static_assert((int)Keys::f10 == VK_F10);
	static_assert((int)Keys::f20 == VK_F20);
	static_assert((int)Keys::f24 == VK_F24);
	static_assert((int)Keys::kp_0 == VK_NUMPAD0);
	static_assert((int)Keys::kp_9 == VK_NUMPAD9);
	static_assert((int)Keys::kp_decimal == VK_DECIMAL);
	static_assert((int)Keys::kp_enter == VK_SEPARATOR);
	static_assert((int)Keys::shift == VK_SHIFT);
	static_assert((int)Keys::control == VK_CONTROL);
	static_assert((int)Keys::alt == VK_MENU);
	static_assert((int)Keys::left_super == VK_LWIN);
	static_assert((int)Keys::right_super == VK_RWIN);
	static_assert((int)Keys::menu == VK_APPS);

	bool down = false;
	switch (message)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			down = true;
			// check if it's repeated events
			// bit 30 is the repeat bit,
			// 1 means previously down
			if (HIWORD(lparam) & 0x4000)
			{
				break;
			}
			// fall through
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			const uint8_t vkcode = LOBYTE(wparam);
			// const uint8_t scancode = (uint8_t)(HIWORD(lparam) & 0xFF);
			// printf("key %s 0x%02x 0x%02x\n", (down ? "down" : "up"), vkcode, scancode);

			InputEvent event;
			event.type = InputEventType::key;
			event.down = down;
			event.key = (Keys)vkcode;
			events.push_back(event);
			break;
		}
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN: down = true;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
			InputEvent event;
			event.type = InputEventType::mouse_button;

            if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP)
                event.button = MouseButtons::left;
            else if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP)
                event.button = MouseButtons::right;
            else if (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP)
                event.button = MouseButtons::middle;
            else if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)
                event.button = MouseButtons::button4;
            else
                event.button = MouseButtons::button5;

            event.down = down;
			events.push_back(event);
			break;
        }
		case WM_MOUSEMOVE:
		{
			InputEvent event;
			event.type = InputEventType::mouse_move;
			event.x = GET_X_LPARAM(lparam);
			event.y = GET_Y_LPARAM(lparam);
			events.push_back(event);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			InputEvent event;
			event.type = InputEventType::mouse_wheel;
			event.delta = (int)std::round(GET_WHEEL_DELTA_WPARAM(wparam) * 100.f / WHEEL_DELTA);
			events.push_back(event);
			break;
		}
	}
}

std::vector<InputEvent> Window::poll_events()
{
	// TODO: change to use the SimpleArray
	std::vector<InputEvent> new_events;

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

		if (msg.message == WM_QUIT)
		{
			request_close = true;
		}
		queue_input_event(new_events, msg.message, msg.wParam, msg.lParam);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return new_events;
}

bool Window::should_close() const 
{
	return request_close;
}

static void get_window_size(const Window& window, int& out_width, int& out_height)
{
	RECT area;
    GetClientRect(get_handle(window), &area);
	out_width = area.right;
	out_height = area.bottom;
}

float Window::aspect() const
{
	int width{}, height{};
	get_window_size(*this, width, height);
	return height > 0 ? (static_cast<float>(width) / height) : 0;
}

int Window::width() const
{
	int width{}, height{};
	get_window_size(*this, width, height);
	return width;
}

int Window::height() const
{
	int width{}, height{};
	get_window_size(*this, width, height);
	return height;
}

void Window::set_title(const String& title)
{
	SetWindowText(get_handle(*this), title.c_str());
}

