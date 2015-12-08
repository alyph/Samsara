// Global constants:
var PLAYGROUND_WIDTH	= 1024;
var PLAYGROUND_HEIGHT	= 768;

var Global = {};

// --------------------------------------------------------------------------------------------------------------------
// --                                      the main declaration:                                                     --
// --------------------------------------------------------------------------------------------------------------------
(function()
{
	function init()
	{
		Archive.init();				
		var game = new Game();
		game.start();				

		(function update()
		{
			game.update();
			window.setTimeout(update, 100);
		})();

		/*
		window.requestAnimationFrame(update);
		function update(timestamp)
		{
			game.update();
			window.requestAnimationFrame(update);
		};*/

		$("#welcomeScreen").fadeTo(500,0,function(){$(this).remove();});
		$("#loadingSet").fadeTo(100,0,function(){$(this).remove();});
	};

	var libs = "Source/Libs/";
	var utils = "Source/Utils/";
	var components = "Source/Components/";
	var definitions = "Source/Definitions/";
	var entities = "Source/Entities/";
	var sequences = "Source/Sequences/";
	var table = "Source/Table/";
	var source = "Source/";
	var core = "Source/Core/";
	var ui = "Source/UI/";

	head.load(
		libs + "jquery-2.0.3.js",
		libs + "jquery-ui-1.8.23.custom.min.js",
		libs + "jquery.gamequery.js",
		libs + "jsface.js",
		utils + "MathEx.js",
		utils + "Containers.js",
		utils + "Utils.js",
		core + "BaseObject.js",
		core + "Components.js",
		core + "Descriptor.js",
		core + "gallery.js",
		core + "Planner.js",
		core + "System.js",
		core + "Archive.js",
		core + "Predicate.js",
		ui + "UI.js",
		ui + "behaviors/basic_behaviors.js",
		// ui + "Elements/BasicElements.js",
		// ui + "Elements/GameElements.js",
		// ui + "Templates/BasicTemplates.js",
		// ui + "Templates/GameTemplates.js",
		// ui + "Templates/Screens.js",
		entities + "Entity.js",
		entities + "Action.js",
		entities + "Actor.js",
		entities + "Message.js",
		entities + "Narrator.js",
		entities + "Card.js",
		entities + "Deck.js",
		entities + "Player.js",
		entities + "Party.js",
		entities + "Character.js",
		entities + "Scene.js",
		entities + "Stage.js",
		entities + "Story.js",
		entities + "Activity.js",
		entities + "Environment.js",
		entities + "Global.js",
		entities + "Keeper.js",
		entities + "Location.js",
		entities + "Goal.js",
		entities + "POV.js",
		entities + "POI.js",
		entities + "Quest.js",
		entities + "World.js",
		entities + "Scripting.js",
		sequences + "Sequence.js",
		sequences + "ActionSequences.js",
		table + "Boards.js",
		table + "CardArea.js",
		table + "CardPiece.js",
		table + "Dialog.js",
		table + "Table.js",
		source + "Game.js",
		source + "Core.js",		
		onCodeLoaded);

	function onCodeLoaded()
	{
		head.load(definitions + "Sprites.js", function()
		{
			Gallery.load(SpriteDefinitions);
			Gallery.ready(onResourceLoaded);
		});
	};

	function onResourceLoaded()
	{
		head.load(
			definitions + "Actions.js",
			definitions + "Keywords.js",
			definitions + "Activities.js",
			definitions + "Characters.js",
			definitions + "Globals.js",
			definitions + "Goals.js",
			definitions + "Environments.js",
			definitions + "Locations.js",
			definitions + "Prototypes.js",
			definitions + "Quests.js",
			definitions + "Scenes.js",			
			definitions + "WorldData.js",
			onDataLoaded);
	};

	function onDataLoaded()
	{
		NewUI.registerTemplates("Source/UI/Templates/basic_templates.html");
		NewUI.registerTemplates("Source/UI/Templates/game_templates.html");
		//NewUI.registerTemplates("Source/UI/Templates/screens.html");

		NewUI.ready(init);
	};



	// head.js(
	// 	libs + "jquery-2.0.3.js",
	// 	libs + "jquery-ui-1.8.23.custom.min.js",
	// 	libs + "jquery.gamequery.js");

	// head.js(
	// 	utils + "MathEx.js",
	// 	utils + "Containers.js",
	// 	utils + "Utils.js");

})();




