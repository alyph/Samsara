#include "common_ui.h"
#include "engine/tablet.h"
#include "engine/enum_utils.h"
#include <cctype>

struct EditBoxState
{
	static const Id type_id;
	LocalString<512> editing_str;
	int cursor{};
};
const Id EditBoxState::type_id = new_state_type_id();

namespace elem
{

	bool edit_box_impl(const Context& ctx, String& text, int width, const EditBoxStyle& style)
	{
		make_element(ctx, null_id);

		_attr(attrs::width, width);
		_attr(attrs::height, 1);

		if (_is_focused)
		{
			_attr(attrs::foreground_color, style.forecolor);
			_attr(attrs::background_color, style.editing_backcolor);

			auto& state = _focused_elem_state(EditBoxState);
			const auto char_code = (char)_char_code;
			if (std::isprint(char_code))
			{
				if (state.editing_str.length < width)
				{
					state.editing_str.chars[state.editing_str.length++] = (char)char_code;
					state.cursor++;
				}
			}
			else if (char_code == ControlCodes::backspace)
			{
				if (state.editing_str.length > 0)
				{
					state.editing_str.length--;
					state.cursor--;
				}
			}

			auto glyphs = make_temp_array<GlyphData>(0, width + 10);
			const auto glyph_color = to_color32(style.forecolor);
			for (int i = 0; i < state.editing_str.length; i++)
			{
				glyphs.push_back(make_glyph(state.editing_str.chars[i], glyph_color, {i, 0}));
			}
			const auto caret_visible = (uint64_t)std::floor(_frame_time * 2) % 2 == 0;
			if (caret_visible)
			{
				glyphs.push_back(make_glyph(0xed, glyph_color, {state.cursor, 0}));
			}
			_attr(attrs::glyphs, glyphs.view());


			if (char_code == ControlCodes::enter)
			{
				_lose_focus();
			}
			else if (char_code == ControlCodes::escape)
			{
				_lose_focus();
				return false;
			}
			if (_will_lose_focus)
			{
				text = state.editing_str.str();
				return true;
			}
		}
		else
		{
			_attr(attrs::text, text);
			_attr(attrs::foreground_color, style.forecolor);
			_attr(attrs::background_color, style.normal_backcolor);

			if (_clicked)
			{
				_gain_focus();
				auto& state = _focused_elem_state(EditBoxState);
				state.editing_str = text;
				state.cursor = (int)text.size();
			}
		}

		return false;
	}


}
