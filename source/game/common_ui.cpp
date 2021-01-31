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
		// glyphs.push_back({.coords={0, -1}, .size={width, 1}, .color2=to_color32(style.editing_backcolor), .code=0xe0});
		// glyphs.push_back({.coords={-1, 0}, .color2=glyph_color, .code='['});
		for (int i = 0; i < state.editing_str.length; i++)
		{
			GlyphData glyph;
			glyph.code = state.editing_str.chars[i];
			// glyph.color1 = to_color32(style.editing_backcolor);
			// glyph.color1 = 0x0055aa_rgb32;
			glyph.color2 = glyph_color;
			glyph.coords = {i, 0};
			glyphs.push_back(glyph);
		}
		// glyphs.push_back({.coords={width, 0}, .color2=glyph_color, .code=']'});
		const auto caret_visible = (uint64_t)std::floor(_frame_time * 2) % 2 == 0;
		auto caret_color = glyph_color;
		caret_color.a = caret_visible ? 255 : 0;
		glyphs.push_back({.coords={state.cursor, 0}, .color2=caret_color, .code=0xed});
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
		if (_will_lose_focused)
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



