#pragma once

enum class InputEventType
{
	key_down,
	key_up,
	mouse_down,
	mouse_up,
	mouse_move,
};

struct InputEvent
{
	InputEventType type{};
	union
	{
		struct
		{
			int key;
			int mods;
		};
		struct
		{
			int button;
		};
		struct
		{
			double x, y;
		};
	};
};