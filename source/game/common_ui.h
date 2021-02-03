#pragma once
#include "engine/presenter.h"

namespace elem
{

	struct EditBoxStyle
	{
		Color forecolor;
		Color normal_backcolor;
		Color editing_backcolor;
	};

	// NOTE: pass ctx as ref since we ensure only one element gets attached to this context chain
	extern bool edit_box_impl(const Context& ctx, String& text, int width, const EditBoxStyle& style);

	static inline bool edit_box(const Context ctx, String& text, int width, const EditBoxStyle& style, Allocator alloc=context_allocator())
	{
		auto local_text = text;
		if (edit_box_impl(ctx, local_text, width, style))
		{
			if (text != local_text)
			{
				text.store(local_text, alloc);
			}
			return true;
		}
		return false;
	}

	template<typename T>
	static inline std::enable_if_t<std::is_integral_v<T>, bool>
	edit_box(const Context ctx, T& num, int width, const EditBoxStyle& style)
	{
		String local_text;
		if constexpr (std::is_signed_v<T>)
		{
			long long d = num;
			local_text = format_str("%lld", d);
		}
		else
		{
			unsigned long long u = num;
			local_text = format_str("%llu", u);
		}
		// TODO: limit the accepted characters
		if (edit_box_impl(ctx, local_text, width, style))
		{
			char* dummy_end;
			// TODO: consider clamping the range
			num = static_cast<T>(std::strtoll(local_text.c_str(), &dummy_end, 10));
			return true;
		}
		return false;
	}

	template<typename T>
	static bool property_table_row(const Context ctx, const String& key, T& val, int key_width, int val_width, const EditBoxStyle& style)
	{
		make_element(ctx, null_id);
		_attr(attrs::width, (key_width + val_width + 1));
		_attr(attrs::height, 1);

		if (_hover)
		{
			_attr(attrs::background_color, 0x00000060_rgba);
		}

		bool ret;
		_children
		{
			// key
			node(_ctx);
			_attr(attrs::text, key);
			_attr(attrs::foreground_color, style.forecolor);
			_attr(attrs::background_color, style.normal_backcolor);
			_attr(attrs::width, (key_width));
			_attr(attrs::height, 1);

			// value
			ret = edit_box(_ctx, val, val_width, style);
			// TODO: we can use a little bit better layouting attributes
			_attr(attrs::placement, ElementPlacement::loose);
			_attr(attrs::left, key_width + 1);
			_attr(attrs::top, 0);

		}
		return ret;
	}

}
