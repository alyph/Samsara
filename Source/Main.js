// Global constants:
var PLAYGROUND_WIDTH	= 1024;
var PLAYGROUND_HEIGHT	= 768;

// --------------------------------------------------------------------------------------------------------------------
// --                                      the main declaration:                                                     --
// --------------------------------------------------------------------------------------------------------------------
$(function()
{

	var scripts =
	[
		"http://cdn.gamequeryjs.com/jquery.gamequery.js",
		"Libs/jsface",
		"Utils/MathEx",
		"Definitions/Cards",
		"Definitions/Decks",
		"Definitions/Sprites",
		"Entities/Card",
		"Entities/Deck",
		"Entities/Player",
		"Game"
	];

	require(scripts, function()
	{
		// Animations declaration:
		Sprites.init();

		// Initialize the game:
		$("#playground").playground({height: PLAYGROUND_HEIGHT, width: PLAYGROUND_WIDTH, keyTracker: true});

		// Initialize the background
		$.playground()
			.addGroup("background", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT})
			.addSprite("backgroundImage", {animation: Sprites.background, width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});

		// this sets the id of the loading bar:
		$.loadCallback(function(percent)
		{
			$("#loadingBar").width(400*percent/100);
		});

		$.playground().startGame(function()
		{
			$("#welcomeScreen").fadeTo(1000,0,function(){$(this).remove();});
			$("#loadingSet").fadeTo(100,0,function(){$(this).remove();});

			var game = new Game();
			game.start();
		});

	});

});

