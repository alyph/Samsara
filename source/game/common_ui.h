#pragma once
#include "engine/presenter.h"

struct EditBoxStyle
{
	Color forecolor;
	Color normal_backcolor;
	Color editing_backcolor;
};

// NOTE: pass ctx as ref since we ensure only one element gets attached to this context chain
extern bool edit_box_impl(const Context& ctx, String& text, int width, const EditBoxStyle& style);

static inline bool edit_box(const Context ctx, String& text, Allocator alloc, int width, const EditBoxStyle& style)
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




