/* globals head, Game */

// --------------------------------------------------------------------------------------------------------------------
// --                                      the main declaration:                                                     --
// --------------------------------------------------------------------------------------------------------------------
(function()
{
	function init()
	{
		//Archive.init();				
		var game = new Game();
		game.start("world");

		// (function update()
		// {
		// 	game.update();
		// 	setTimeout(update, 100);
		// })();

		document.querySelector("#welcomeScreen").classList.add("hidden");

		/*
		window.requestAnimationFrame(update);
		function update(timestamp)
		{
			game.update();
			window.requestAnimationFrame(update);
		};*/

		//$("#welcomeScreen").fadeTo(500,0,function(){$(this).remove();});
		//$("#loadingSet").fadeTo(100,0,function(){$(this).remove();});
	}

	var libs = "Source/Libs/";
	var utils = "Source/Utils/";
	// var components = "Source/Components/";
	// var definitions = "Source/Definitions/";
	var gameplay = "Source/gameplay/";
//	var sequences = "Source/Sequences/";
//	var table = "Source/Table/";
	var source = "Source/";
	var core = "Source/Core/";
	var ui = "Source/UI/";

	head.load(
		//libs + "jquery-2.0.3.js",
		//libs + "jquery-ui-1.8.23.custom.min.js",
		//libs + "jquery.gamequery.js",
		libs + "jsface.js",
		utils + "MathEx.js",
		utils + "Containers.js",
		utils + "Utils.js",
		utils + "GLP.js",
		core + "BaseObject.js",
		core + "Components.js",
		//core + "Descriptor.js",
		core + "events.js",
		//core + "gallery.js",
		core + "Planner.js",
		core + "System.js",
		core + "Archive.js",
		core + "Predicate.js",
		ui + "UI.js",
		ui + "directives/basic_directives.js",
		ui + "proxies/game_proxies.js",
		//ui + "behaviors/basic_behaviors.js",
		//ui + "behaviors/game_behaviors.js",
		// ui + "Elements/BasicElements.js",
		// ui + "Elements/GameElements.js",
		// ui + "Templates/BasicTemplates.js",
		// ui + "Templates/GameTemplates.js",
		// ui + "Templates/Screens.js",
		gameplay + "world.js",
		gameplay + "entity.js",
		gameplay + "action.js",
		gameplay + "area.js",
		gameplay + "attribute.js",
		gameplay + "card.js",
		gameplay + "descriptor.js",
		gameplay + "effect.js",
		gameplay + "encounter.js",
		gameplay + "field.js",
		gameplay + "global.js",
		gameplay + "mechanics.js",
		gameplay + "player.js",
		gameplay + "rule.js",
		gameplay + "scripting.js",
		gameplay + "sprite.js",
		gameplay + "trait.js",
		gameplay + "trigger.js",
		// sequences + "Sequence.js",
		// sequences + "ActionSequences.js",
		// table + "Boards.js",
		// table + "CardArea.js",
		// table + "CardPiece.js",
		// table + "Dialog.js",
		// table + "Table.js",
		source + "Game.js",
		source + "Core.js",		
		onCodeLoaded);

	// function onCodeLoaded()
	// {
	// 	head.load(definitions + "Sprites.js", function()
	// 	{
	// 		Gallery.load(SpriteDefinitions);
	// 		Gallery.ready(onResourceLoaded);
	// 	});
	// }

	// function onResourceLoaded()
	// {
	// 	head.load(
	// 		//definitions + "Actions.js",
	// 		//definitions + "Keywords.js",
	// 		//definitions + "Activities.js",
	// 		//definitions + "Characters.js",
	// 		definitions + "Cards.js",
	// 		definitions + "Globals.js",
	// 		//definitions + "Goals.js",
	// 		//definitions + "Environments.js",
	// 		//definitions + "Locations.js",
	// 		//definitions + "Prototypes.js",
	// 		//definitions + "Quests.js",
	// 		//definitions + "Scenes.js",			
	// 		definitions + "WorldData.js",
	// 		onDataLoaded);
	// }

	function onCodeLoaded()
	{
		Archive.init({ baseUrl: "Content/data" });
		let templatesLoaded =
		[
			Archive.loadInstances("world/world"),
			UI.registerTemplates("Source/UI/Templates/basic_templates.html"),
			UI.registerTemplates("Source/UI/views/player_view.html"),
			UI.registerTemplates("Source/UI/views/card_view.html")
		];
		
		Promise.all(templatesLoaded)
			.then(v => { UI.reportUnfinishedTemplates(); })
			.then(init, onFailed);
	}

	function onFailed(error)
	{
		console.error(`Game failed to initialize: ${error.message}\n${error.stack}`);
	}

	// head.js(
	// 	libs + "jquery-2.0.3.js",
	// 	libs + "jquery-ui-1.8.23.custom.min.js",
	// 	libs + "jquery.gamequery.js");

	// head.js(
	// 	utils + "MathEx.js",
	// 	utils + "Containers.js",
	// 	utils + "Utils.js");

})();




