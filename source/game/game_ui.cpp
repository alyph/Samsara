#include "game_ui.h"
#include "engine/tablet.h"

using namespace elem;


static Color card_color(const CardType& type)
{
	switch (type.group)
	{
		case CardGroup::regular: return {0.9f, 0.9f, 0.75f, 1.0f};
		case CardGroup::event: return {0.9f, 0.85f, 0.55f, 1.0f};
		case CardGroup::stratagem: return {0.65f, 0.3f, 0.85f, 1.0f};
		case CardGroup::project: return {0.35f, 0.95f, 0.65f, 1.0f};
		case CardGroup::unit: return {1.0f, 0.4f, 0.45f, 1.0f};
		case CardGroup::innovation: return {0.4f, 0.7f, 0.95f, 1.0f};
		default: return {1.f, 1.f, 1.f, 1.f};
	}
}



static void card_view(const Context ctx, const Card& card, const GameRef& ref, CardId& out_clicked_card)
{
	asserts(card.id);
	const auto type_id = card.type;
	const auto& type = ref.globals.card_types.get(type_id);
	const auto& fore_color = card_color(type);

	String flag;
	// if (ref.world.phase == TurnPhase::action)
	// {
	// 	if (is_playing_card(ref.world.player, card.id))
	// 	{
	// 		status = " [destined]";
	// 	}
	// }
	if (is_ephemeral_card(ref.world.player, card.id))
	{
		flag = "[E] ";
	}


	node(_ctx);
	_attr(attrs::height, 2);

	if (_hover)
	{
		const uint16_t hover_mark = 0x01ba;//'>';
		auto glyphs = make_temp_array<GlyphData>(0, 4);
		glyphs.push_back(make_glyph(hover_mark, to_color32(fore_color), {1, 0}));
		_attr(attrs::glyphs, glyphs.view());
	}

	if (_clicked)
	{
		out_clicked_card = card.id;
	}

	_children
	{
		node(_ctx);
		const auto phrase = format_str("   ... %s%s", flag, type.phrase);
		_attr(attrs::text, phrase);
		_attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);
	}
}

static void click_card(const GameRef& ref, CardId card_id)
{
	const auto phase = ref.world.phase;
	if (phase == TurnPhase::action)
	{
		if (is_playing_card(ref.world.player, card_id))
		{
			unplay_card(ref, card_id);
		}
		else
		{
			play_card(ref, card_id);
		}
	}
}

static String get_year_season(const GameRef& ref)
{
	const auto& world = ref.world;
	String seasons[] = {"Spring", "Summer", "Autumn", "Winter"};
	const auto year = world.starting_year + (world.turn / 4);
	const auto season = (world.turn % 4);
	return format_str("%s of Year %d", seasons[season], year);
}

void game_ui(const Context ctx, const GameRef& ref, float vp_width, float vp_height, const TabletAsset& tablet_asset)
{
	// TODO: styling parameters should be passed in (preferably as a style collection)
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


	CardId clicked_card_id = 0;

	_children
	{
		node(_ctx);
		_attr(attrs::height, 2);

		node(_ctx);
		// String str = "In the year 1198 of God's grace";
		_attr(attrs::text, get_year_season(ref));
		_attr(attrs::foreground_color, fore_color);

		node(_ctx);
		_attr(attrs::height, 1);

		node(_ctx);
		// String str = "The people of Outremer enjoyed the peaceful time.";
		_attr(attrs::text, "This is a peaceful time, under God's gracious watch.");
		_attr(attrs::foreground_color, fore_color);

		// node(_ctx);
		// _attr(attrs::height, 1);

		// node(_ctx);
		// // str = "The cities were growing and farms were ploughed.";
		// _attr(attrs::text, "The cities were growing and farms were ploughed.");
		// _attr(attrs::foreground_color, fore_color);

		node(_ctx);
		_attr(attrs::height, 2);

		node(_ctx);
		// str = "Now the pages are turning, and the future unfolds...";
		_attr(attrs::text, "Now the pages are turning, and the future unfolds...");
		_attr(attrs::foreground_color, fore_color);

		node(_ctx);
		_attr(attrs::height, 1);


		auto& player = ref.world.player;
		for (size_t i = 0; i < player.hand.size(); i++)
		{
			const auto& card = player.hand[i];
			card_view(_ctx_id(card.id), card, ref, clicked_card_id);
			// const auto card_type = player.hand[i].type;

			// node(_ctx_id(i));
			// str = format_str("   ... %s", ref.globals.card_types.get(card_type).phrase);
			// _attr(attrs::text, str);
			// _attr(attrs::foreground_color, fore_color);

			// node(_ctx_id(i));
			// _attr(attrs::height, 1);
		}

		// node(_ctx);
		// _attr(attrs::height, 1);

		// for (size_t i = 0; i < player.regular_hand.size(); i++)
		// {
		// 	const auto& card = player.regular_hand[i];
		// 	card_view(_ctx_id(card.id), card, ref, clicked_card_id);
		// 	// const auto card_type = player.regular_hand[i].type;

		// 	// node(_ctx_id(i));
		// 	// str = format_str("   ... %s", ref.globals.card_types.get(card_type).phrase);
		// 	// _attr(attrs::text, str);
		// 	// _attr(attrs::foreground_color, fore_color);

		// 	// node(_ctx_id(i));
		// 	// _attr(attrs::height, 1);
		// }

		// node(_ctx);
		// _attr(attrs::height, 1);


		// node(_ctx);
		// _attr(attrs::text, format_str("    Accounts: %d / %d", player.actions.size(), player.action_size));
		// _attr(attrs::foreground_color, fore_color);



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

	if (clicked_card_id)
	{
		click_card(ref, clicked_card_id);
	}
}



