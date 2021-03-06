#include "game.h"
#include "common_ui.h"
#include "game_ui.h"
#include "game_utils.h"
#include "engine/window.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/image_utils.h"
#include "engine/mesh.h"
#include "engine/filesystem.h"
#include "engine/font.h"
#include "engine/random.h"
#include "easy/profiler.h"

// static constexpr const Vec2i initial_map_vp{ (map_chunk_size / 2), (map_chunk_size / 2) };
// TODO: hard code this vp for now, later should load from user session data
static constexpr const Vec2i initial_map_vp{ 400, 250 };
static const String main_scenario_dir = "../../data/game/scenarios/main";

struct GameModel
{
	const Globals& globals;
	World& world;
	GameState& game;
	EditorState& editor;
};

static void start_game(const GameRef& ref);
static void start_turn(const GameRef& ref);
static void start_draft_phase(const GameRef& ref);
static void start_action_phase(const GameRef& ref);
static void start_resolution_phase(const GameRef& ref);
static void next_turn(const GameRef& ref);

static bool is_regular_card(const Globals& globals, Id card_type);

Game::Game()
{
	// TODO: move to asset manager or something
	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	tablet_shader = create_shader(desc);

	desc.vs_path = "../../data/shaders/basic_textured_vs.gls";
	desc.fs_path = "../../data/shaders/basic_textured_fs.gls";
	tablet_screen_shader = create_shader(desc);

	// auto tex_desc = load_texture("../../data/fonts/Anikki_square_32x32.png");
	// atlas_texture = Texture::create(tex_desc);
	atlas_texture = load_texture_array(
	{
		"../../data/fonts/cp437_24x24.png",
		// "../../data/fonts/anikki_square_24x24.png",
		"../../data/fonts/nice_curses_24x24.png",
		"../../data/fonts/urr_a_24x24.png",
		"../../data/fonts/path_24x24.png",
	});

	desc.vs_path = "../../data/shaders/mesh_vs.gls";
	desc.fs_path = "../../data/shaders/mesh_fs.gls";
	const auto ref_map_shader = create_shader(desc);

	// TODO: just one pixel per tile, will adjust it later if needed
	const float half_w = 3648 / 4; // note: integer
	const float half_h = 3004 / 4;
	const Color mesh_tint{ 1.f, 1.f, 1.f, 0.3f };
	Mesh mesh;
	mesh.indices = { 0, 1, 2, 0, 2, 3 };
	mesh.vertices = 
	{
		{ -half_w, -half_h, 0 }, { half_w, -half_h, 0 }, 
		{ half_w, half_h, 0 }, { -half_w, half_h, 0 },
	};
	mesh.colors = 
	{
		mesh_tint, mesh_tint, mesh_tint, mesh_tint, 
	};
	mesh.uvs =
	{
		{ 0.f, 0.f }, { 1.f, 0.f }, { 1.f, 1.f }, { 0.f, 1.f },
	};
	ref_map_mesh = add_mesh(std::move(mesh), ref_map_shader);
	ref_map_texture = load_texture("../../data/images/europe1054ref.png");

	// const auto font = create_font("../../data/fonts/unscii-16.ttf", 0, 44);
	// const int w{16}, h{16}, pages{1};
	// const int num = (w * h * pages);
	// ArrayTemp<uint32_t> codes{(size_t)num};
	// for (int i = 0; i < num; i++)
	// {
	// 	codes[i] = i;
	// }
	// ui_texture = create_font_texture(w, h, pages, codes, font);
	ui_texture = load_texture_array(
	{
		"../../data/fonts/unscii_16_ascii.png",
		"../../data/fonts/unscii_16_box.png",
		// "../../data/fonts/unscii_8x16.png",
		// "../../data/fonts/Lord_Nightmare-Fixedsys-03.png",
	});

	// const auto font = create_font("../../data/fonts/unscii-16-full.ttf", 0, 16);
	// create_font_glyph_page(0x0, font);

	const auto make_dev_type = [&](const String& struct_key, const String& name, DevelopmentArea area, uint16_t glyph, const Color32& color) -> DevelopmentType&
	{
		auto& dev = globals.development_types[(int)area] = {name, area, 0};
		const auto struct_flags = StructureFlags::dev | (area == DevelopmentArea::urban ? StructureFlags::urban : StructureFlags::rural);
		auto& structure = globals.structure_types.set(struct_key, {0, name, glyph, color, struct_flags, (Id)area});
		dev.structure_type = structure.id;
		return dev;
	};

	const auto alloc = app_allocator();

	globals.terrain_types.alloc(64, alloc);
	globals.terrain_types.set(0, "ocean", {0, "ocean", '~', 0x0300, 0x103060_rgb32, 0x80c0f0_rgb32, TileTypeFlags::water});
	globals.terrain_types.set("clear", {0, "clear", '.', 0, 0_rgb32, 0_rgb32});
	globals.terrain_types.set("forest", {0, "forest", 'T', 0x0105, 0x30ff50_rgb32, 0x30ff50_rgb32});
	globals.terrain_types.set("hill", {0, "hill", '^', 0x0218, 0x606070_rgb32, 0x606070_rgb32});

	globals.structure_types.alloc(1024, alloc);
	globals.structure_types.set(0, "none", {0, "none", 0, 0_rgb32, StructureFlags::none, 0});

	make_dev_type("dev_urban", "urban", DevelopmentArea::urban, 0x007f, 0xa09060_rgb32);
	make_dev_type("dev_rural", "rural", DevelopmentArea::rural, 0x007f, 0x50b080_rgb32);

	const auto& wall_struct = globals.structure_types.set("wall", {0, "wall", 0x0320, 0xafafaf_rgb32, StructureFlags::wall, 0});
	const auto& urban_vacancy = globals.structure_types.set("vacancy", {0, "vacancy", 0x00f9, 0x503010_rgb32, StructureFlags::urban|StructureFlags::vacancy, 0});

	const auto make_card = [&](const String& key, const String& name, const String& phrase, unsigned int weight, CardGroup group)
	{
		return globals.card_types.set(key, {.name = name, .phrase = phrase, .weight = weight, .group = group});
	};

	const auto make_regular_card = [&](const String& key, const String& name, const String& phrase)
	{
		return make_card(key, name, phrase, 0, CardGroup::regular);
	};

	const auto make_event_card = [&](const String& key, const String& name, const String& phrase)
	{
		return make_card(key, name, phrase, 120, CardGroup::event);
	};

	const auto make_stratagem_card = [&](const String& key, const String& name, const String& phrase)
	{
		return make_card(key, name, phrase, 80, CardGroup::stratagem);
	};

	const auto make_project_card = [&](const String& key, const String& name, const String& phrase)
	{
		return make_card(key, name, phrase, 20, CardGroup::project);
	};

	const auto make_unit_card = [&](const String& key, const String& name, const String& phrase)
	{
		return make_card(key, name, phrase, 20, CardGroup::unit);
	};

	const auto make_innovation_card = [&](const String& key, const String& name, const String& phrase)
	{
		return make_card(key, name, phrase, 20, CardGroup::innovation);
	};



	globals.card_types.alloc(4096, alloc);
	const auto& card_economy = make_regular_card("reg_economy", "Economy", "a pursuit of wealth");
	const auto& card_military = make_regular_card("reg_military", "Military", "a pursuit of power");
	const auto& card_innovation = make_regular_card("reg_innovation", "Innovation", "a pursuit of ideas");
	const auto& card_diplomacy = make_regular_card("reg_diplomacy", "Diplomacy", "a pursuit of influence");
	const auto& card_statecraft = make_regular_card("reg_statecraft", "statecraft", "a pursuit of order");
	const auto& card_intrigue = make_regular_card("reg_intrigue", "Intrigue", "a pursuit of shadow");
	const auto& card_destiny = make_regular_card("reg_destiny", "Destiny", "a pursuit of destiny");

	make_card("war", "War", "a war!", 80, CardGroup::event);
	make_event_card("relation_improve", "Relationship Improved", "a warming relationship");
	make_event_card("relation_deteriorate", "Relationship Deteriorated", "a growing tension");

	make_stratagem_card("stg_alliance", "Form Alliance", "a new ally");
	make_stratagem_card("stg_trade_agreement", "Trade Agreement", "a trade agreement");
	make_stratagem_card("stg_marriage", "Marriage Proposal", "a marriage proposal");
	make_stratagem_card("stg_incite_war", "Incite War", "a word of war");
	make_stratagem_card("stg_incite_division", "Sow Division", "a word of division");
	make_stratagem_card("stg_incite_discontent", "Sow Discontent", "a word of resentment");
	make_stratagem_card("stg_incite_treachery", "Incite Treachery", "a word of betrayal");
	make_stratagem_card("stg_incite_coup", "Incite Coup", "a word of treason");
	make_stratagem_card("stg_spy_network", "Spy Network", "a web of whispers");

	make_project_card("prj_farm", "Farmstead", "expansion of farmlands");
	make_project_card("prj_pasture", "Pasture", "expansion of pastures");
	make_project_card("prj_forestry", "Forestry", "expansion of forestries");
	make_project_card("prj_hunting", "Hunting Ground", "expansion of hunting grounds");
	make_project_card("prj_orchard", "Orchard", "expansion of orchards");
	make_project_card("prj_vineyard", "Vineyard", "expansion of vineyards");
	make_project_card("prj_fishery", "Fishery", "expansion of fisheries");
	make_project_card("prj_quarry", "Quarry", "expansion of quarries");
	make_project_card("prj_mine", "Mine", "expansion of mining network");
	make_project_card("prj_irrigation", "Irrigation System", "construction of irrigation system");

	make_project_card("prj_harbor", "Harbor", "expansion of harbor");
	make_project_card("prj_market", "Market", "expansion of market");
	make_project_card("prj_tradecenter", "Trade Center", "development of trade center");
	make_project_card("prj_masonry", "Masonry", "development of masonry");
	make_project_card("prj_ironworks", "Ironworks Manufacture", "development of ironworks manufacture");
	make_project_card("prj_glassworks", "Glassworks Manufacture", "development of glassworks manufacture");
	make_project_card("prj_artisan", "Artisan Manufacture", "development of artisan manufacture");
	make_project_card("prj_textile", "Textile Manufacture", "development of artisan manufacture");
	make_project_card("prj_jewelry", "Jewelry Manufacture", "development of jewelry manufacture");
	make_project_card("prj_weapons", "Weapons Manufacture", "development of weapons manufacture");
	make_project_card("prj_shipyard", "Shipyard", "construction of shipyard");
	make_project_card("prj_cathedral", "Cathedral", "construction of cathedral");
	make_project_card("prj_theatre", "Theatre", "construction of theatre");
	make_project_card("prj_palace", "Palace", "construction of palace");
	make_project_card("prj_monastery", "Monastery", "construction of grand monastary");
	make_project_card("prj_roads", "Road Network", "development of road network");
	make_project_card("prj_walls", "City Wall", "construction of city wall");
	make_project_card("prj_reserves", "Reserves", "development of reserves");

	make_project_card("prj_outpost", "Outpost", "construction of outpost"); // simple fortification near the frontier
	make_project_card("prj_castle", "Castle", "construction of castle");
	make_project_card("prj_fortress", "Castle", "construction of fortress"); // high level castle
	make_project_card("prj_camp", "Army Camp", "construction of army camp");
	make_project_card("prj_base", "Army Base", "construction of army base"); // high level camp
	make_project_card("prj_naval_base", "Naval Base", "construction of naval base");

	make_unit_card("unit_slingers", "slingers", "recruitment of slingers");
	make_unit_card("unit_javelinmen", "Javelinmen", "recruitment of javelinmen");
	make_unit_card("unit_skirmishers", "Javelinmen", "recruitment of javelinmen");
	make_unit_card("unit_archers", "Archers", "recruitment of archers");
	make_unit_card("unit_light_archers", "Light Archers", "recruitment of light archers");
	make_unit_card("unit_light_crossbowmen", "Light Crossbowmen", "recruitment of light crossbowmen");
	make_unit_card("unit_crossbowmen", "Crossbowmen", "recruitment of crossbowmen");
	make_unit_card("unit_armoured_crossbowmen", "Armoured Crossbowmen", "recruitment of armoured crossbowmen");
	make_unit_card("unit_light_spearmen", "Light Spearmen", "recruitment of light spearmen");
	make_unit_card("unit_spearmen", "Spearmen", "recruitment of spearmen");
	make_unit_card("unit_armoured_spearmen", "Armoured Spearmen", "recruitment of armoured spearmen");
	make_unit_card("unit_militia_spearmen", "Militia Spearmen", "recruitment of militia spearmen");
	make_unit_card("unit_armoured_militia", "Armoured Militia", "recruitment of armoured militia");
	make_unit_card("unit_horse_archers", "Horse Archers", "recruitment of horse archers");
	make_unit_card("unit_mounted_crossbowmen", "Mounted Crossbowmen", "recruitment of mounted crossbowmen");
	make_unit_card("unit_knights", "Knights", "recruitment of knights");
	make_unit_card("unit_sergeant ", "Sergeant", "recruitment of sergeants");
	make_unit_card("unit_armoured_cavalry", "Armoured Cavalry", "recruitment of armoured cavalry");
	make_unit_card("unit_cavalry", "Cavalry", "recruitment of cavalry");

	make_innovation_card("innov_monasticism", "Monasticism", "rise of monasticism");
	make_innovation_card("innov_manorialism", "Manorialism", "rise of manorialism");
	make_innovation_card("innov_feudalism", "Feudalism", "rise of feudalism");
	make_innovation_card("innov_nobility", "Nobility", "rise of nobility");
	make_innovation_card("innov_gavelkind", "Gavelkind", "adoption of gavelkind");
	make_innovation_card("innov_primogeniture", "Primogeniture", "adoption of primogeniture");
	make_innovation_card("innov_retinues", "Retinues", "adoption of retinues");
	make_innovation_card("innov_knighthood", "Knighthood", "rise of knights");
	make_innovation_card("innov_chivalry", "Chivalry", "rise of chivalry");
	make_innovation_card("innov_heraldry", "Heraldry", "adoption of heraldry");
	make_innovation_card("innov_scutage", "Scutage", "adoption of scutage");
	make_innovation_card("innov_municipal_officials", "Municipal Officials", "establishment of municipal officials");
	make_innovation_card("innov_assemblies", "Assemblies", "adoption of assemblies"); // Witenagemot for saxons?
	make_innovation_card("innov_state_admin", "State Administration", "conception of state administration"); // inception? rise? ideas?
	make_innovation_card("innov_chronicle", "Chronicle", "development of chronicle writing");
	make_innovation_card("innov_renaissance_arts", "Renaissance Arts", "rise of the renaissance arts");

	make_innovation_card("innov_two_field", "Two-field System", "adoption of two-field system");
	make_innovation_card("innov_three_field", "Three-field System", "development of three-field system");
	make_innovation_card("innov_hourseshoes", "Horseshoes", "invention of horseshoes");
	make_innovation_card("innov_onagers", "Onagers", "invention of onagers");
	make_innovation_card("innov_mangonels", "Mangonels", "invention of mangonels");
	make_innovation_card("innov_trebuchet", "Trebuchet", "invention of trebuchet");
	make_innovation_card("innov_bombards", "Bombards", "invention of bombards");
	make_innovation_card("innov_castle", "Castle", "development of castle constructions");
	make_innovation_card("innov_chainmail", "Chainmail", "development of chainmail");
	make_innovation_card("innov_plate_armour", "Plate Armour", "development of plate armour");
	make_innovation_card("innov_minting", "Minting", "development of minting");
	make_innovation_card("innov_windmills", "Windmills", "development of windmills");
	make_innovation_card("innov_banking", "Banking", "rise of banking");
	make_innovation_card("innov_guilds", "Guilds", "rise of guilds");
	make_innovation_card("innov_church_shooling", "Church Schooling", "rise of church schooling");
	make_innovation_card("innov_crane", "Crane", "invention of crane");
	make_innovation_card("innov_tournament", "Tournament", "rise of tournament");
	make_innovation_card("innov_gunpowder", "Gunpowder", "adoption of gunpowder");
	make_innovation_card("innov_oil_painting", "Oil Painting", "development of oil painting");
	make_innovation_card("innov_armillary", "Armillary Sphere", "invention of armillary sphere");



	globals.defines.starting_wall_type = wall_struct.id;
	globals.defines.urban_vacancy_type = urban_vacancy.id;
	globals.defines.starting_deck.alloc(alloc, 
	{
		{card_economy.id, 		1},
		{card_military.id, 		1},
		{card_innovation.id, 	1},
		{card_diplomacy.id, 	1},
		{card_statecraft.id, 	1},
		{card_intrigue.id, 		1},
		{card_destiny.id, 		1},
	});




	editor_state.map_vp = initial_map_vp;
	// TODO: this is the legacy problem, we made the map based on this initial ref image offset
	// later we should have ways to shift map as well as the ref images
	editor_state.ref_image_offset = { (map_chunk_size / 2), -(map_chunk_size / 2) };
	
	const auto game_alloc = stage_allocator();

	if (filesystem::exists(main_scenario_dir))
	{
		world = load_world(main_scenario_dir, globals, game_alloc);
	}
	else
	{
		world.alloc(game_alloc);
	}

	game_state.in_editor = false;
	editor_state.selected_brush = (int)Brush::selection;

	// initialize card pools
	// TODO: for now just add all non-regular card into the pool
	game_state.player_card_pool.alloc(0, globals.card_types.array.size(), game_alloc);
	for (const auto& card_type : globals.card_types)
	{
		if (!is_regular_card(globals, card_type.id))
		{
			game_state.player_card_pool.push_back({.card_type = card_type.id, .weight = card_type.weight});
		}
	}


	// if turn < 0, it means the game hasn't started yet
	if (world.turn < 0)
	{
		start_game({game_state, world, globals});
	}
}

void Game::update(const Time& time)
{
#if 0
	if (!game_state.in_editor)
	{
		timer += dt;
		if (timer >= 0.1)
		{
			timer = 0.0;

			for (auto& city : world.cities)
			{
				if (rand_int(6) < 4)
				{
					const auto area = rand_int(6) < 2 ? DevelopmentArea::urban : DevelopmentArea::rural;
					develop_city(world, city.id, area, globals);
				}
			}
		}
	}
#endif
}


static uint32_t used_card_id = 0;

static inline Card make_card(Id card_type_id)
{
	return { .id = (++used_card_id), .type = static_cast<decltype(Card::type)>(card_type_id) };
}

static Array<CardId> draft_cards_from_pool(const GameRef& ref, int count)
{
	auto cards = make_temp_array<CardId>(count);

	// TODO: do we want any duplicated draft???
	// some can be (like recruitment, projects, missions)
	// some not (like innovations)
	for (int i = 0; i < count; i++)
	{
		const auto& entry = random_weighted_array_element(ref.game.player_card_pool);
		const auto card = make_card(entry.card_type);
		cards[i] = card.id;
		ref.world.player.hand.push_back(card);
	}
	return cards;
}

static void discard_card(const GameRef& ref, CardId id)
{
	auto& player = ref.world.player;
	for (size_t i = 0; i < player.hand.size(); i++)
	{
		const auto& card = player.hand[i];
		if (card.id == id)
		{
			// regular cards go back to regular deck, other cards are destroyed
			if (is_regular_card(ref.globals, card.type))
			{
				player.regular_deck.push_back(card);
			}
			player.hand.erase(i);
			break;
		}
	}
}

static bool is_regular_card(const Globals& globals, Id card_type)
{
	for (const auto& starting_card : globals.defines.starting_deck)
	{
		if (starting_card.type == card_type)
		{
			return true;
		}
	}
	return false;
};



static void start_game(const GameRef& ref)
{
	// fill the player's starting deck
	for (const auto& startingCard : ref.globals.defines.starting_deck)
	{
		for (uint16_t i = 0; i < startingCard.count; i++)
		{
			ref.world.player.regular_deck.push_back(make_card(startingCard.type));
		}
	}

	ref.world.turn = 0; // first turn
	start_turn(ref);
}


static void start_turn(const GameRef& ref)
{
	if (ref.world.turn % 4 == 0)
	{
		start_draft_phase(ref);
	}
	else
	{
		start_action_phase(ref);
	}
}

static void start_draft_phase(const GameRef& ref)
{
	ref.world.phase = TurnPhase::draft;

	// TODO: stuff
	const int hand_limit = 8;
	const int draft_count = std::clamp((ref.world.turn == 0) ? 4: 2, 0, hand_limit - (int)ref.world.player.hand.size());
	if (draft_count > 0)
	{
		draft_cards_from_pool(ref, draft_count);
	}
	start_action_phase(ref);
}

static void start_action_phase(const GameRef& ref)
{
	ref.world.phase = TurnPhase::action;

	// draw cards from the deck to hand
	// TODO: if we have discard pile, then we need dump discarded cards back to deck when it gets low
	auto& player = ref.world.player;

	const int num_ephemeral_cards = 2;
	const int num_regular_cards = 2;

	// draft ephemeral cards
	const auto ephemerals = draft_cards_from_pool(ref, num_ephemeral_cards);

	// draw regular cards
	const auto regulars = draw_random_cards(player.regular_deck, player.hand, 2);

	player.ephemeral_cards.concat(ephemerals);
	player.ephemeral_cards.concat(regulars);
}


static void start_resolution_phase(const GameRef& ref)
{
	ref.world.phase = TurnPhase::resolution;

	// discard all ephemeral cards
	auto& player = ref.world.player;
	for (const auto eph_card_id : player.ephemeral_cards)
	{
		discard_card(ref, eph_card_id);
	}
	player.ephemeral_cards.clear();

	// start the next turn
	next_turn(ref);
}

static void next_turn(const GameRef& ref)
{
	ref.world.turn++;
	start_turn(ref);
}


bool has_card_in_hand(const Player& player, CardId id)
{
	for (const auto& card : player.hand)
	{
		if (card.id == id)
		{
			return true;
		}
	}
	return false;
}

bool play_card(const GameRef& ref, CardId id)
{
	auto& player = ref.world.player;
	if (!has_card_in_hand(player, id))
	{
		return false;
	}
	// TODO: execute actions
	discard_card(ref, id);
	start_resolution_phase(ref);
	return true;
}

bool unplay_card(const GameRef& ref, CardId id)
{
	auto& player = ref.world.player;
	size_t idx = -1;
	for (size_t i = 0; i < player.actions.size(); i++)
	{
		if (player.actions[i].card_id == id)
		{
			idx = i;
			break;
		}
	}
	if (idx != -1)
	{
		player.actions.erase(idx);
		return true;
	}
	return false;
}

bool is_playing_card(const Player& player, CardId card_id)
{
	for (const auto& action : player.actions)
	{
		if (action.card_id == card_id)
		{
			return true;
		}
	}
	return false;
}

bool is_ephemeral_card(const Player& player, CardId card_id)
{
	for (const auto& eph_card_id : player.ephemeral_cards)
	{
		if (eph_card_id == card_id)
		{
			return true;
		}
	}
	return false;

}




static void paint_square(Map& map, Id terrain_type, const Vec2i& center, int radius, const Globals& globals)
{
	// TODO: move this function to map and let it do less glyph update
	int r = (radius - 1);
	for (int dx = -r; dx <= r; dx++)
	{
		for (int dy = -r; dy <= r; dy++)
		{
			map.set_terrain(center + Vec2i{dx, dy}, terrain_type);
		}
	}
	map.update_glyphs({center - Vec2i{r, r}, center + Vec2i{r, r}}, globals);
}

static void paint_line(Map& map, Id terrain_type, const Vec2i& start, const Vec2i& end, int radius, const Globals& globals)
{
	// https://en.wikipedia.org/wiki/Bresenham's_line_algorithm#All_cases
	// http://members.chello.at/~easyfilter/Bresenham.pdf

	const auto dx = std::abs(end.x - start.x);
	const auto dy = -std::abs(end.y - start.y);
	const auto sx = (start.x < end.x ? 1 : -1);
	const auto sy = (start.y < end.y ? 1 : -1);
	auto err = (dx + dy);
	auto x = start.x;
	auto y = start.y;
	paint_square(map, terrain_type, {x, y}, radius, globals);
	while (!((x == end.x && y == end.y)))
	{
		const auto e2 = err * 2;
		if (e2 >= dy) { err += dy; x += sx; }
		if (e2 <= dx) { err += dx; y += sy; }
		// TODO: if this gets too expensive, can just draw the deltas
		paint_square(map, terrain_type, {x, y}, radius, globals);
	}
}

static void draw_border(const Box2i& box, Color32 border_color, Color32 back_color, TabletRenderBuffer& render_buffer, Id elem_id)
{
	GlyphData glyph;
	glyph.color1 = back_color;
	glyph.color2 = border_color;
	auto draw = [&](uint16_t code, const Vec2i& coords, const Vec2i& size)
	{
		glyph.code = code;
		glyph.coords = coords;
		glyph.size = size;
		render_buffer.push_glyph(elem_id, glyph);
	};

	// border
	const int w = (box.max.x - box.min.x + 1);
	const int h = (box.max.y - box.min.y + 1);
	asserts(w >= 2 && h >= 2);
	uint16_t border_codes[] = { 0x00da, 0x00c4, 0x00bf, 0x00b3, 0x00d9, 0x00c0 };
	draw(border_codes[0], box.min, {1, 1});
	draw(border_codes[2], {box.max.x, box.min.y}, {1, 1});
	draw(border_codes[4], box.max, {1, 1});
	draw(border_codes[5], {box.min.x, box.max.y}, {1, 1});
	if (w > 2)
	{
		draw(border_codes[1], {box.min.x + 1, box.min.y}, {w - 2, 1});
		draw(border_codes[1], {box.min.x + 1, box.max.y}, {w - 2, 1});
	}
	if (h > 2)
	{
		draw(border_codes[3], {box.max.x, box.min.y + 1}, {1, h - 2});
		draw(border_codes[3], {box.min.x, box.min.y + 1}, {1, h - 2});
	}
}

static const constexpr uint16_t sel_brush_glyph = 0x02f1;
static const constexpr Color32 sel_brush_color = 0xffffff_rgb32;
static const constexpr uint16_t city_brush_glyph = 0x02e1;
static const constexpr Color32 city_brush_color = 0xffff00_rgb32;

static inline Vec2i to_tablet_coords(const Vec2i& map_coords, const TabletLayout& layout, const Vec2i& map_min_coords)
{
	return { layout.left + map_coords.x - map_min_coords.x, layout.top + map_coords.y - map_min_coords.y };
}

static Id map_view(const Context ctx, World& world, const Globals& globals, bool in_editor, EditorState& state, int cols, int rows, const GameModel& model)
{
	EASY_FUNCTION();

	const Id elem_id = make_element(ctx, null_id);
	_attr(attrs::width, cols);
	_attr(attrs::height, rows);

	TabletLayout layout;
	TabletRenderBuffer& render_buffer = access_tablet_render_buffer_and_layout(ctx, layout);
	int map_x = state.map_vp.x - layout.width / 2;
	int map_y = state.map_vp.y - layout.height / 2;
	const Vec2i map_min{map_x, map_y};

	Map& map = world.map;
	const auto& default_terrain = globals.terrain_types.get(null_id);
	TileGlyph default_glyph;
	default_glyph.code = default_terrain.glyph;
	default_glyph.color = default_terrain.color_a;
	for (int y = 0; y < layout.height; y++)
	{
		for (int x = 0; x < layout.width; x++)
		{		
			Vec2i tile_coords{ map_x + x, map_y + y };
			Id tile_id = map.tile_id(tile_coords);
			const auto& ground_glyph = (tile_id && id_to_index(tile_id) < map.ground_glyphs.size()) ?
				map.ground_glyphs[id_to_index(tile_id)] : default_glyph;

			if (ground_glyph.code > 0)
			{
				GlyphData glyph;
				glyph.code = ground_glyph.code;
				glyph.color2 = ground_glyph.color;
				glyph.coords = { layout.left + x, layout.top + y };
				render_buffer.push_glyph(elem_id, glyph);
			}
		}
	}

	const int cell_x_min = tile_to_cell(map_x);
	const int cell_x_max = tile_to_cell(map_x + layout.width - 1);
	const int cell_y_min = tile_to_cell(map_y);
	const int cell_y_max = tile_to_cell(map_y + layout.height - 1);

	for (int cell_y = cell_y_min; cell_y <= cell_y_max; cell_y++)
	{
		const int shift = (cell_y % 2 == 0) ? 0 : 1;
		const int cell_x_begin = cell_x_min / 2 * 2 + shift;
		const int cell_x_end = cell_x_max / 2 * 2 + shift;
		const int x_begin = cell_x_begin * map_cell_size - map_x + layout.left;
		const int x_end = cell_x_end * map_cell_size - map_x + layout.left;
		const int y = cell_y * map_cell_size - map_y + layout.top;
		for (int x = x_begin; x <= x_end; x += (map_cell_size * 2))
		{
			GlyphData glyph;
			glyph.code = 0;
			glyph.color1 = 0xffffff10_rgba32;
			glyph.coords = { x, y };
			glyph.size = {map_cell_size, map_cell_size};
			render_buffer.push_glyph(elem_id, glyph);
		}
	}

	if (state.selected_city_id)
	{
		const auto& city = world.cities.get(state.selected_city_id);
		auto bounds = city.urban_bounds;
		pad_box(bounds, 1);
		Box2i border_box
		{
			to_tablet_coords(bounds.min, layout, map_min),
			to_tablet_coords(bounds.max, layout, map_min),
		};
		draw_border(border_box, 0xff2020e0_rgba32, 0xc0_rgba32, render_buffer, elem_id);
	}

	model.game.hover_over_map = false;
	if (_hover)
	{
		const Vec2i& cursor = ctx.frame->curr_input.mouse_hit.iuv;
		if (cursor.x >= layout.left && cursor.x < (layout.left + layout.width) &&
			cursor.y >= layout.top && cursor.y < (layout.top + layout.height))
		{
			const Vec2i map_cursor{map_x + cursor.x - layout.left, map_y + cursor.y - layout.top};
			model.game.hover_over_map = true;
			model.game.hover_over_map_pos = map_cursor;

			if (!in_editor || (state.selected_brush == (int)Brush::selection))
			{
				if (_pressed)
				{
					for (const auto& city : world.cities)
					{
						if (encompasses(city.urban_bounds, map_cursor))
						{
							state.selected_city_id = city.id;
							break;
						}
					}
				}
			}
			else
			{
				uint16_t code{};
				Color32 color{};
				Vec2i center{cursor.x, cursor.y};
				int r{};

				int terrain_brush = (state.selected_brush - (int)Brush::max);
				if (terrain_brush >= 0 /* && terrain_brush < globals.terrain_types.size()*/)
				{
					const TerrainType& tile_type = globals.terrain_types.get(terrain_brush);
					code = tile_type.glyph;
					color = tile_type.color_a;
					r = state.brush_radius - 1;

					if (_down)
					{
						if (!state.painting)
						{
							state.painting = true;
							state.paint_cursor = map_cursor;
							paint_square(map, terrain_brush, map_cursor, state.brush_radius, globals);
						}
						else if (state.paint_cursor != map_cursor)
						{
							paint_line(map, terrain_brush, state.paint_cursor, map_cursor, state.brush_radius, globals);
							state.paint_cursor = map_cursor;
						}				
					}
				}
				else if (state.selected_brush == (int)Brush::city)
				{
					// city
					code = city_brush_glyph;
					if (valid_urban_tile(map, map_cursor, globals))
					{
						color = city_brush_color;
						if (_pressed)
						{
							create_city(world, map_cursor, globals);
						}
					}
					else
					{
						color = 0xff0000_rgb32;
					}
				}

				const int d = r * 2 + 1;
				GlyphData glyph;
				glyph.code = code;
				glyph.color2 = color;
				glyph.color1 = 0x303030_rgb32;
				glyph.coords = { center.x - r, center.y - r };
				glyph.size = { d, d };
				render_buffer.push_glyph(elem_id, glyph);
			}

			if (!state.dragging_map && _right_down)
			{
				state.dragging_map = true;
				state.dragging_map_coord = map_cursor;
				const auto& ruv = ctx.frame->curr_input.mouse_hit.ruv;
				state.dragging_map_offset = {ruv.x - cursor.x, ruv.y - cursor.y};
			}
		}
	}

	// TODO: we won't be able to clear this state if we are not rendered
	// have a reliable way from the presenter for initializing and cleaning up
	// states, maybe this should be the internal states of the elements?
	// another way is asking the presenter if this element was rendered last frame
	// or similarly whether an initialization action should run because element
	// wasn't there last frame
	if (!_down)
	{
		state.painting = false;
	}

	return elem_id;
}

static int palette_button(const Context ctx, int index, int begin_row, uint16_t glyph_code, const Color32& color, int val, int& selected_val)
{
	int size = 3;
	int cols = 2;

	int row = begin_row + (index / cols);
	int col = (index % cols);

	const Id button_id = make_element(ctx, null_id);
	_attr(attrs::placement, ElementPlacement::loose);
	_attr(attrs::left, col * size);
	_attr(attrs::top, row * size);
	_attr(attrs::width, size);
	_attr(attrs::height, size);

	const bool selected = (selected_val == val);

	// TODO: we should make a helper to get both render buffer and layout
	// otherwise user can cache a render_buffer outside of the loop and reuse it
	// while the layout has not been properly finalized
	TabletLayout layout;
	TabletRenderBuffer& render_buffer = access_tablet_render_buffer_and_layout(ctx, layout);
	GlyphData glyph;
	int x{layout.left}, y{layout.top};

	auto draw = [&](uint16_t code, const Vec2i& coords)
	{
		glyph.code = code;
		glyph.coords = coords;
		render_buffer.push_glyph(button_id, glyph);
	};

	// TODO: maybe allow pushing a static array of glyphs
	// center glyph
	glyph.color1 = 0xd0_rgba32; // darkened translucent background
	glyph.color2 = color;
	draw(glyph_code, {x + 1, y + 1});

	uint16_t normal_border[] = { 0x00da, 0x00c4, 0x00bf, 0x00b3, 0x00d9, 0x00c0 };
	uint16_t selected_border[] = { 0x00c9, 0x00cd, 0x00bb, 0x00ba, 0x00bc, 0x00c8 };
	uint16_t* border_codes = selected? selected_border : normal_border;

	// border
	glyph.color2 = 0xd0d0d0_rgb32;
	draw(border_codes[0], {x, y});
	draw(border_codes[1], {x + 1, y});
	draw(border_codes[2], {x + 2, y});
	draw(border_codes[3], {x + 2, y + 1});
	draw(border_codes[4], {x + 2, y + 2});
	draw(border_codes[1], {x + 1, y + 2});
	draw(border_codes[5], {x, y + 2});
	draw(border_codes[3], {x, y + 1});

	// happen next frame
	if (_clicked)
	{
		selected_val = val;
	}

	return row;
}

static void tile_palette(const Context ctx, const Collection<TerrainType>& terrain_types, EditorState& state, int& row)
{
	int begin_row = row;
	int idx = 0;
	for (const auto& type : terrain_types)
	{
		row = palette_button(_ctx_id(idx), idx, begin_row, type.glyph, type.color_a, (int)Brush::max + (int)type.id, state.selected_brush);
		idx++;
	}
}

static void brush_size_palette(const Context ctx, EditorState& state, int& row)
{
	int begin_row = row;
	int brush_sizes[] = {1, 2, 4, 8};
	for (int i = 0; i < LEN(brush_sizes); i++)
	{
		const int size = brush_sizes[i];
		uint16_t code = ('0' + size);
		row = palette_button(_ctx_id(i), i, begin_row, code, 0xffffff_rgb32, size, state.brush_radius);
	}
}

static void city_dev_palette(const Context ctx, Id sel_city, const Globals& globals, World& world, int& row)
{
	int begin_row = row;
	int sel_dev_type = -1;
	int idx = 0;
	for (const auto& dev_type : globals.development_types)
	{
		const StructureType& struct_type = globals.structure_types.get(dev_type.structure_type);
		row = palette_button(_ctx_id(idx), idx, begin_row, struct_type.glyph, struct_type.color, (int)dev_type.area, sel_dev_type);
		idx++;
	}

	if (sel_dev_type >= 0)
	{
		develop_city({world, globals}, sel_city, (DevelopmentArea)sel_dev_type, 1);
	}
}

void Game::present(const Context& ctx)
{
	using namespace elem;

	auto window = engine().window;

	const float aspect = window->aspect();

	const int tablet_rows = 80;
	const float tablet_cell_size = 1.f;
	const float tablet_height = tablet_rows * tablet_cell_size;

	const float vp_width = tablet_height * aspect;
	const float vp_height = tablet_height;

	const int ui_rows = 64;
	const float ui_cell_w = calc_tablet_width(1, ui_rows, tablet_height, ui_texture);
	const float ui_cell_h = calc_tablet_height(1, 1, ui_cell_w, ui_texture);

	Color fore_color{0.8f, 0.8f, 1.f, 1.f};

	GameModel model{ globals, world, game_state, editor_state };
	WorldRef world_ref{ world, globals };

	// most of the allocation goes to the current world
	// TODO: after we have different screens (e.g. main menu, campaign etc.),
	// we will need change the allocator based on where we are
	const auto world_alloc = world.allocator;
	const auto selected_city = editor_state.selected_city_id; // use the same one through out the frame even if it's changed in the middle

	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);

	viewport(_ctx);
	_attr(attrs::width, window->width());
	_attr(attrs::height, window->height());
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, (Color{0.f, 0.f, 0.f, 1.f}));

	if (_key_pressed(Keys::grave_accent, ModKeys::ctrl))
	{
		game_state.in_editor = !game_state.in_editor;
	}

	_children
	{
		// map view
		float min_zoom = 0.25f;
		float max_zoom = 2.0;
		const int map_rows = (int)std::round(tablet_rows * max_zoom) + 4;;
		const int map_cols = (map_rows * 2);
		editor_state.map_scale += _mouse_wheel_delta * 0.001f;
		const auto map_scale = editor_state.map_scale = std::clamp(editor_state.map_scale, 1/max_zoom, 1/min_zoom);
		
		// TODO: code below and several other places will not work if the map cells are not square
		// it should employ a more generalized algorithem:
		// 1. calculate the new map tablet position based on the new mouse position and real number cell coordinates (ruv?) it's dragging on 
		// 2. offset this new map position with the new map_vp, so the map position never goes over one cell
		// 3. other overlay images (like the ref map) should follow the map position calculated in step 1
		if (editor_state.dragging_map)
		{
			if (is_mouse_down(ctx, MouseButtons::right))
			{
				const auto& input = ctx.frame->curr_input;
				Vec2d ndc = calc_ndc({input.mouse_x, input.mouse_y}, window->width(), window->height());
				double world_x = ndc.x * vp_width / (2 * map_scale) - editor_state.dragging_map_offset.x;
				double world_y = ndc.y * vp_height / (2 * map_scale) + editor_state.dragging_map_offset.y;
				int half_cols = map_cols / 2;
				int half_rows = map_rows / 2;
				// TODO: world_x, y needs to be divided by tablet_cell_size to get translated to tablet cell coordinates
				int tablet_x = std::clamp((int)std::floor(world_x), -half_cols, half_cols-1);
				int tablet_y = std::clamp((int)std::floor(world_y), -half_rows, half_rows-1);
				// TODO: since we are changing the map pose here, it will actually invalidate the iuv hit cell done by previous raycast
				// which is stored in the curr_input.mouse_hit. When the later code is using that iuv (like in the map_view())
				// the values there are no longer valid and will have 1 frame delay, so ideally we should calculate the hit 
				// map coordinates right there, store and use it whenever needed consistently
				editor_state.map_pose_offset = {(float)(world_x - tablet_x), (float)(world_y - tablet_y)};
				editor_state.map_vp = (editor_state.dragging_map_coord + Vec2i{-tablet_x, tablet_y});
			}
			else
			{
				editor_state.dragging_map = false;
			}
		}

		tablet(_ctx);
		
		Pose map_pose;
		map_pose.pos.x = editor_state.map_pose_offset.x;
		map_pose.pos.y = editor_state.map_pose_offset.y;
		map_pose.pos.z = 1.f;
		const auto map_scale_mat = make_mat44_scale({map_scale, map_scale, 1.f});
		_attr(attrs::transform, map_scale_mat * to_mat44(map_pose));
		_attr(attrs::width, map_cols * tablet_cell_size);
		_attr(attrs::height, map_rows * tablet_cell_size);
		_attr(attrs::tablet_columns, map_cols);
		_attr(attrs::tablet_rows, map_rows);
		_attr(attrs::texture, atlas_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			map_view(_ctx, world, globals, game_state.in_editor, editor_state, map_cols, map_rows, model);
		}

		if (game_state.in_editor)
		{
			// reference image
			mesh(_ctx);
			Pose mesh_pose;
			auto vp_offset = to_vec2(editor_state.map_vp);
			vp_offset.y = -vp_offset.y;
			mesh_pose.pos = to_vec3(editor_state.map_pose_offset - vp_offset + editor_state.ref_image_offset, 0.5);
			_attr(attrs::mesh_id, ref_map_mesh);
			_attr(attrs::transform, map_scale_mat * to_mat44(mesh_pose));
			_attr(attrs::texture, ref_map_texture);

			// tool box
			tablet(_ctx);
			const int tools_cols = 6;
			const float tools_width = (tools_cols * tablet_cell_size);
			Pose overlay_tablet_pose;
			overlay_tablet_pose.pos.x = (vp_width - tools_width) / 2; // anchored to the right
			_attr(attrs::transform, to_mat44(overlay_tablet_pose));
			_attr(attrs::width, tools_width);
			_attr(attrs::height, tablet_height);
			_attr(attrs::tablet_columns, tools_cols);
			_attr(attrs::tablet_rows, tablet_rows);
			_attr(attrs::texture, atlas_texture);
			_attr(attrs::shader, tablet_shader);
			_attr(attrs::quad_shader, tablet_screen_shader);

			_children
			{
				int row = 0;
				tile_palette(_ctx, globals.terrain_types, editor_state, row); 
				row++;
				brush_size_palette(_ctx, editor_state, row);
				row++;
				palette_button(_ctx, 0, row, sel_brush_glyph, sel_brush_color, (int)Brush::selection, editor_state.selected_brush);
				row = palette_button(_ctx, 1, row, city_brush_glyph, city_brush_color, (int)Brush::city, editor_state.selected_brush) + 1;
				if (editor_state.selected_city_id)
				{
					row++;
					city_dev_palette(_ctx, editor_state.selected_city_id, globals, world, row);
					row++;
				}
			}

			// contextual property window
			const bool show_properties_window = (editor_state.selected_city_id);
			if (show_properties_window)
			{
				scoped_context_allocator(world_alloc); // all data stored in the world allocator

				tablet(_ctx);
				const int props_cols = 50;
				const float props_cols_width = calc_tablet_width(props_cols, ui_rows, tablet_height, ui_texture);
				Pose props_tablet_pose;
				props_tablet_pose.pos.x = -(vp_width - props_cols_width) / 2;
				_attr(attrs::transform, to_mat44(props_tablet_pose));
				_attr(attrs::width, props_cols_width);
				_attr(attrs::height, tablet_height);
				_attr(attrs::tablet_columns, props_cols);
				_attr(attrs::tablet_rows, ui_rows);
				_attr(attrs::texture, ui_texture);
				_attr(attrs::shader, tablet_shader);
				_attr(attrs::quad_shader, tablet_screen_shader);
				_attr(attrs::background_color, (Color{0.f, 0.f, 0.f, 0.5f}));

				_children
				{
					node(_ctx);
					_attr(attrs::placement, ElementPlacement::loose);
					_attr(attrs::left, 1);
					_attr(attrs::top, 1);

					_children
					{
						EditBoxStyle edit_style;
						edit_style.forecolor = fore_color;
						edit_style.normal_backcolor = 0_rgba;
						edit_style.editing_backcolor = 0x111122_rgb;
						int key_w = 20;
						int val_w = 24;

						if (selected_city)
						{
							// TODO: if selected city changed, we need make sure all edits are commited to the old city
							// Although right now it's kinda automatic since the mouse input that triggers the city selection
							// will make the edit box lose focus, which then commits changes here
							auto& city = world.cities.get(selected_city);
							edit_box(_ctx, city.name, 48, edit_style);

							node(_ctx);
							_attr(attrs::height, 1);

							if (property_table_row(_ctx, "Urban Level", city.development_level[(int)DevelopmentArea::urban], key_w, val_w, edit_style))
							{
								reset_city_development(world_ref, city.id, DevelopmentArea::urban, city.development_level[(int)DevelopmentArea::urban]);
							}
							if (property_table_row(_ctx, "Rural Level", city.development_level[(int)DevelopmentArea::rural], key_w, val_w, edit_style))
							{
								reset_city_development(world_ref, city.id, DevelopmentArea::rural, city.development_level[(int)DevelopmentArea::rural]);
							}
						}
					}
				}
			}
		}
		else
		{
			GameRef game_ref{ game_state, world, globals };
			TabletAsset tablet_asset{ ui_texture, tablet_shader, tablet_screen_shader };

			game_ui(_ctx, game_ref, vp_width, vp_height, tablet_asset);

#if 0
			const int ui_left_cols = 100;
			const float ui_left_width = calc_tablet_width(ui_left_cols, ui_rows, tablet_height, ui_texture);

			tablet(_ctx);
			Pose ui_tablet_pose;
			ui_tablet_pose.pos.x = -(vp_width - ui_left_width) / 2;
			_attr(attrs::transform, to_mat44(ui_tablet_pose));
			_attr(attrs::width, ui_left_width);
			_attr(attrs::height, tablet_height);
			_attr(attrs::tablet_columns, ui_left_cols);
			_attr(attrs::tablet_rows, ui_rows);
			_attr(attrs::texture, ui_texture);
			_attr(attrs::shader, tablet_shader);
			_attr(attrs::quad_shader, tablet_screen_shader);

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

				node(_ctx);
				str = "          ... A marriage proposal";
				_attr(attrs::text, str);
				_attr(attrs::foreground_color, fore_color);

				node(_ctx);
				_attr(attrs::height, 1);

				node(_ctx);
				str = "          ... An emerging farmstead";
				_attr(attrs::text, str);
				_attr(attrs::foreground_color, fore_color);

				node(_ctx);
				_attr(attrs::height, 1);

				node(_ctx);
				str = "          ... knights seeking employment";
				_attr(attrs::text, str);
				_attr(attrs::foreground_color, fore_color);

				node(_ctx);
				_attr(attrs::height, 1);

				node(_ctx);
				str = "          ... A ruse";
				_attr(attrs::text, str);
				_attr(attrs::foreground_color, fore_color);

				node(_ctx);
				_attr(attrs::height, 1);
			}
#endif
		}

		const int ui_full_cols = 400; // TODO: we can probably calculate this based on the widest aspect ratio we want to support, or we could add support for mutable sized tablets
		const int ui_bottom_rows = 2;

		const float ui_bottom_w = ui_cell_w * ui_full_cols;
		const float ui_bottom_h = ui_cell_h * ui_bottom_rows;

		tablet(_ctx);
		Pose ui_bottom_pose;
		ui_bottom_pose.pos.y = -(vp_height - ui_bottom_h) / 2;
		_attr(attrs::transform, to_mat44(ui_bottom_pose));
		_attr(attrs::width, ui_bottom_w);
		_attr(attrs::height, ui_bottom_h);
		_attr(attrs::tablet_columns, ui_full_cols);
		_attr(attrs::tablet_rows, ui_bottom_rows);
		_attr(attrs::texture, ui_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);
		_attr(attrs::background_color, (Color{0.f, 0.f, 0.f, 0.5f}));

		_children
		{
			int visible_ui_cols = (int)std::floor(vp_width / ui_cell_w);
			int left = std::max(0, (ui_full_cols - visible_ui_cols) / 2) + 1;
			int top = 0;

			auto map_pos_str = format_str("(%d, %d)", game_state.hover_over_map_pos.x, game_state.hover_over_map_pos.y);

			node(_ctx);
			_attr(attrs::placement, ElementPlacement::loose);
			_attr(attrs::left, left);
			_attr(attrs::top, top);
			_attr(attrs::foreground_color, fore_color);
			_attr(attrs::text, map_pos_str);

			// node(_ctx);
			// _attr(attrs::placement, ElementPlacement::loose);
			// _attr(attrs::left, left + 16);
			// _attr(attrs::top, top);
			// _attr(attrs::foreground_color, fore_color);
			// _attr(attrs::text, format_str("%.3f", _frame_time));
		}
	}
}

bool Game::ended()
{
	return engine().window->should_close();
}

void Game::shutdown()
{
	filesystem::create_directories(main_scenario_dir);
	save_world(world, main_scenario_dir, globals);
}

