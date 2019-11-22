#pragma once
#include <cstdint>

enum class InputEventType
{
	key,
	mouse_button,
	mouse_move,
};

enum class MouseButtons : uint8_t
{
	button1 = 0,
	button2,
	button3,
	button4,
	button5,
	button6,
	button7,
	button8,
	max,
	left = button1,
	right = button2,
	middle = button3,
};

enum class Keys : uint8_t
{
	// mostly just follow the windows virtual key codes
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	space              = 0x20,

	num0               = 0x30,
	num1               = 0x31,
	num2               = 0x32,
	num3               = 0x33,
	num4               = 0x34,
	num5               = 0x35,
	num6               = 0x36,
	num7               = 0x37,
	num8               = 0x38,
	num9               = 0x39,

	a                  = 0x41,
	b                  = 0x42,
	c                  = 0x43,
	d                  = 0x44,
	e                  = 0x45,
	f                  = 0x46,
	g                  = 0x47,
	h                  = 0x48,
	i                  = 0x49,
	j                  = 0x4A,
	k                  = 0x4B,
	l                  = 0x4C,
	m                  = 0x4D,
	n                  = 0x4E,
	o                  = 0x4F,
	p                  = 0x50,
	q                  = 0x51,
	r                  = 0x52,
	s                  = 0x53,
	t                  = 0x54,
	u                  = 0x55,
	v                  = 0x56,
	w                  = 0x57,
	x                  = 0x58,
	y                  = 0x59,
	z                  = 0x5A,

	semicolon          = 0xBA,  /* ; */
	equal              = 0xBB,  /* = */
	comma              = 0xBC,  /* , */
	minus              = 0xBD,  /* - */
	period             = 0xBE,  /* . */
	slash              = 0xBF,  /* / */
	grave_accent       = 0xC0,  /* ` */

	left_bracket       = 0xDB,  /* [ */
	backslash          = 0xDC,  /* \ */
	right_bracket      = 0xDD,  /* ] */
	quote         	   = 0xDE,  /* ' */

	escape             = 0x1B,
	enter              = 0x0D,
	tab                = 0x09,
	backspace          = 0x08,
	insert             = 0x2D,
	del                = 0x2E,
	right              = 0x27,
	left               = 0x25,
	down               = 0x28,
	up                 = 0x26,
	page_up            = 0x21,
	page_down          = 0x22,
	home               = 0x24,
	end                = 0x23,
	caps_lock          = 0x14,
	scroll_lock        = 0x91,
	num_lock           = 0x90,
	print_screen       = 0x2C,
	cancel             = 0x03,
	pause              = 0x13,
	
	f1                 = 0x70,
	f2                 = 0x71,
	f3                 = 0x72,
	f4                 = 0x73,
	f5                 = 0x74,
	f6                 = 0x75,
	f7                 = 0x76,
	f8                 = 0x77,
	f9                 = 0x78,
	f10                = 0x79,
	f11                = 0x7A,
	f12                = 0x7B,
	f13                = 0x7C,
	f14                = 0x7D,
	f15                = 0x7E,
	f16                = 0x7F,
	f17                = 0x80,
	f18                = 0x81,
	f19                = 0x82,
	f20                = 0x83,
	f21                = 0x84,
	f22                = 0x85,
	f23                = 0x86,
	f24                = 0x87,

	kp_0               = 0x60,
	kp_1               = 0x61,
	kp_2               = 0x62,
	kp_3               = 0x63,
	kp_4               = 0x64,
	kp_5               = 0x65,
	kp_6               = 0x66,
	kp_7               = 0x67,
	kp_8               = 0x68,
	kp_9               = 0x69,

	kp_decimal         = 0x6E,
	kp_divide          = 0x6F,
	kp_multiply        = 0x6A,
	kp_subtract        = 0x6D,
	kp_add             = 0x6B,
	kp_enter           = 0x6C,

	shift              = 0x10,
	control            = 0x11,
	alt                = 0x12,
	// not using the left/right versions for now
	// left_shift         = 0xA0,
	// left_control       = 0xA2,
	// left_alt           = 0xA4,
	// right_shift        = 0xA1,
	// right_control      = 0xA3,
	// right_alt          = 0xA5,
	left_super         = 0x5B,
	right_super        = 0x5C,
	menu               = 0x5D,
	max,
};


struct InputEvent
{
	InputEventType type{};
	union
	{
		struct
		{
			Keys key;
			bool down;
		};
		struct
		{
			MouseButtons button;
			// share 'down' with the key event
		};
		struct
		{
			double x, y;
		};
	};
};