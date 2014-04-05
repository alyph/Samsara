// Global constants:
var PLAYGROUND_WIDTH	= 1024;
var PLAYGROUND_HEIGHT	= 768;

// --------------------------------------------------------------------------------------------------------------------
// --                                      the main declaration:                                                     --
// --------------------------------------------------------------------------------------------------------------------
(function()
{
	head.ready(function()
	{
		// Animations declaration:
		Sprites.init();

		// Initialize the game:
		$("#playground").playground({height: PLAYGROUND_HEIGHT, width: PLAYGROUND_WIDTH, keyTracker: true});

		// Initialize the background
		$.playground()
			.addGroup("background", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT})
			/*.addSprite("backgroundImage", {animation: Sprites.background, width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT})*/;

		// this sets the id of the loading bar:
		$.loadCallback(function(percent)
		{
			$("#loadingBar").width(400*percent/100);
		});

		$.playground().startGame(function()
		{
			Core.init();

			var game = new Game();
			game.start();

			$("#welcomeScreen").fadeTo(1000,0,function(){$(this).remove();});
			$("#loadingSet").fadeTo(100,0,function(){$(this).remove();});
		});

	});

	var libs = "Source/Libs/";
	var utils = "Source/Utils/";
	var components = "Source/Components/";
	var definitions = "Source/Definitions/";
	var entities = "Source/Entities/";
	var table = "Source/Table/";
	var source = "Source/";
	var core = "Source/Core/";

	head.js(
		libs + "jsface.js",
		core + "Components.js",
		core + "Planner.js",
		core + "System.js",
		entities + "Entity.js",
		entities + "Message.js",
		entities + "Narrator.js",
		entities + "Card.js",
		entities + "Deck.js",
		entities + "Player.js",
		entities + "Party.js",
		entities + "Scene.js",
		entities + "Stage.js",
		entities + "Story.js",
		entities + "Activity.js",
		entities + "Location.js",
		entities + "Goal.js",
		entities + "Quest.js",
		entities + "World.js",
		table + "Boards.js",
		table + "CardArea.js",
		table + "CardPiece.js",
		table + "Dialog.js",
		table + "Table.js",
		source + "Game.js",
		source + "Core.js",
		definitions + "Activities.js",
		definitions + "Characters.js",
		definitions + "Goals.js",
		definitions + "Locations.js",
		definitions + "Quests.js",
		definitions + "Sprites.js",
		definitions + "WorldData.js");

	head.js(
		libs + "jquery-2.0.3.js",
		libs + "jquery-ui-1.8.23.custom.min.js",
		libs + "jquery.gamequery.js");

	head.js(
		utils + "MathEx.js",
		utils + "Utils.js");

})();




