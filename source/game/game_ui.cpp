#include "game_ui.h"
#include "engine/tablet.h"

using namespace elem;

void game_ui(const Context ctx, GameRef& ref, float vp_width, float vp_height, const TabletAsset& tablet_asset)
{
	// TODO: styling parameters should be passed in (preferablly as a style collection)
	Color fore_color{0.8f, 0.8f, 1.f, 1.f};
	Color back_color(0, 0, 0, 0.8f);

	const int ui_rows = 64;
	const int ui_left_cols = 64;
	const float ui_left_width = calc_tablet_width(ui_left_cols, ui_rows, vp_height, tablet_asset.glyph_texture);

	tablet(_ctx);
	Pose ui_tablet_pose;
	ui_tablet_pose.pos.x = -(vp_width - ui_left_width) / 2;
	_attr(attrs::transform, to_mat44(ui_tablet_pose));
	_attr(attrs::width, ui_left_width);
	_attr(attrs::height, vp_height);
	_attr(attrs::tablet_columns, ui_left_cols);
	_attr(attrs::tablet_rows, ui_rows);
	_attr(attrs::texture, tablet_asset.glyph_texture);
	_attr(attrs::shader, tablet_asset.tablet_shader);
	_attr(attrs::quad_shader, tablet_asset.screen_shader);
	_attr(attrs::background_color, back_color);

	_children
	{
		node(_ctx);
		_attr(attrs::height, 2);

		node(_ctx);
		String str = "In the year 1198 of God's grace";
		_attr(attrs::text, str);
		_attr(attrs::foreground_color, fore_color);

		node(_ctx);
		_attr(attrs::height, 1);

		node(_ctx);
		str = "The people of Outremer enjoyed the peaceful time.";
		_attr(attrs::text, str);
		_attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);

		node(_ctx);
		str = "The cities were growing and farms were ploughed.";
		_attr(attrs::text, str);
		_attr(attrs::foreground_color, fore_color);

		node(_ctx);
		_attr(attrs::height, 2);

		node(_ctx);
		str = "Now the pages are turning, and the future unfolds...";
		_attr(attrs::text, str);
		_attr(attrs::foreground_color, fore_color);

		node(_ctx);
		_attr(attrs::height, 1);


		auto& player = ref.world.player;
		for (size_t i = 0; i < player.hand.size(); i++)
		{
			const auto card_type = player.hand[i].type;

			node(_ctx_id(i));
			str = format_str("   ... %s", ref.globals.card_types.get(card_type).phrase);
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, fore_color);

			node(_ctx_id(i));
			_attr(attrs::height, 1);
		}

		node(_ctx);
		_attr(attrs::height, 1);

		for (size_t i = 0; i < player.regular_hand.size(); i++)
		{
			const auto card_type = player.regular_hand[i].type;

			node(_ctx_id(i));
			str = format_str("   ... %s", ref.globals.card_types.get(card_type).phrase);
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, fore_color);

			node(_ctx_id(i));
			_attr(attrs::height, 1);
		}




		// node(_ctx);
		// str = "          ... A marriage proposal";
		// _attr(attrs::text, str);
		// _attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);

		// node(_ctx);
		// str = "          ... An emerging farmstead";
		// _attr(attrs::text, str);
		// _attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);

		// node(_ctx);
		// str = "          ... knights seeking employment";
		// _attr(attrs::text, str);
		// _attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);

		// node(_ctx);
		// str = "          ... A ruse";
		// _attr(attrs::text, str);
		// _attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);
	}
}



