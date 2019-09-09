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
	space              = 32,
	apostrophe         = 39,  /* ' */
	comma              = 44,  /* , */
	minus              = 45,  /* - */
	period             = 46,  /* . */
	slash              = 47,  /* / */
	num0               = 48,
	num1               = 49,
	num2               = 50,
	num3               = 51,
	num4               = 52,
	num5               = 53,
	num6               = 54,
	num7               = 55,
	num8               = 56,
	num9               = 57,
	semicolon          = 59,  /* ; */
	equal              = 61,  /* = */
	a                  = 65,
	b                  = 66,
	c                  = 67,
	d                  = 68,
	e                  = 69,
	f                  = 70,
	g                  = 71,
	h                  = 72,
	i                  = 73,
	j                  = 74,
	k                  = 75,
	l                  = 76,
	m                  = 77,
	n                  = 78,
	o                  = 79,
	p                  = 80,
	q                  = 81,
	r                  = 82,
	s                  = 83,
	t                  = 84,
	u                  = 85,
	v                  = 86,
	w                  = 87,
	x                  = 88,
	y                  = 89,
	z                  = 90,
	left_bracket       = 91,  /* [ */
	backslash          = 92,  /* \ */
	right_bracket      = 93,  /* ] */
	grave_accent       = 96,  /* ` */


	escape             = 128,
	enter              = 129,
	tab                = 130,
	backspace          = 131,
	insert             = 132,
	del                = 133,
	right              = 134,
	left               = 135,
	down               = 136,
	up                 = 137,
	page_up            = 138,
	page_down          = 139,
	home               = 140,
	end                = 141,
	caps_lock          = 152,
	scroll_lock        = 153,
	num_lock           = 154,
	print_screen       = 155,
	pause              = 156,
	f1                 = 162,
	f2                 = 163,
	f3                 = 164,
	f4                 = 165,
	f5                 = 166,
	f6                 = 167,
	f7                 = 168,
	f8                 = 169,
	f9                 = 170,
	f10                = 171,
	f11                = 172,
	f12                = 173,
	f13                = 174,
	f14                = 175,
	f15                = 176,
	f16                = 177,
	f17                = 178,
	f18                = 179,
	f19                = 180,
	f20                = 181,
	f21                = 182,
	f22                = 183,
	f23                = 184,
	f24                = 185,
	f25                = 186,
	kp_0               = 192,
	kp_1               = 193,
	kp_2               = 194,
	kp_3               = 195,
	kp_4               = 196,
	kp_5               = 197,
	kp_6               = 198,
	kp_7               = 199,
	kp_8               = 200,
	kp_9               = 201,
	kp_decimal         = 202,
	kp_divide          = 203,
	kp_multiply        = 204,
	kp_subtract        = 205,
	kp_add             = 206,
	kp_enter           = 207,
	kp_equal           = 208,
	left_shift         = 212,
	left_control       = 213,
	left_alt           = 214,
	left_super         = 215,
	right_shift        = 216,
	right_control      = 217,
	right_alt          = 218,
	right_super        = 219,
	menu               = 220,
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