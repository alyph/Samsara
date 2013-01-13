// Global constants:
var PLAYGROUND_WIDTH	= 1024;
var PLAYGROUND_HEIGHT	= 768;

// --------------------------------------------------------------------------------------------------------------------
// --                                      the main declaration:                                                     --
// --------------------------------------------------------------------------------------------------------------------
$(function()
{
	require.config(
	{
		paths:
		{
			jqueryui: 'Libs/jquery-ui-1.8.23.custom.min',
			gamequery: 'Libs/jquery.gamequery' //"http://cdn.gamequeryjs.com/jquery.gamequery.js"
		}

	});

	var libs =
	[
		"jquery",
		"jqueryui",
		"gamequery",
		"Libs/jsface",
		"Utils/MathEx"
	];

	var scripts =
	[
		"Components/CardComponent",
		"Entities/Table",
		"Entities/Message",
		"Entities/Card",
		"Entities/Deck",
		"Entities/Player",
		"Entities/Party",
		"Entities/Stage",
		"Entities/Story",
		"Entities/Activity",
		"Core",
		"Game"
	];

	var definitions =
	[
		"Components/Types",
		"Definitions/Activities",
		"Definitions/Cards",
		"Definitions/Companies",
		"Definitions/Encounters",
		"Definitions/Sprites",
		"Definitions/Stages",
		"Definitions/Stories"

	];

	require(libs, function()
	{
		require(scripts, function()
		{
			require(definitions, function()
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
					Core.init();

					var game = new Game();
					game.start();

					$("#welcomeScreen").fadeTo(1000,0,function(){$(this).remove();});
					$("#loadingSet").fadeTo(100,0,function(){$(this).remove();});
				});

			});
		});
	});
});

