// Global constants:
var PLAYGROUND_WIDTH	= 800;
var PLAYGROUND_HEIGHT	= 600;

// --------------------------------------------------------------------------------------------------------------------
// --                                      the main declaration:                                                     --
// --------------------------------------------------------------------------------------------------------------------
$(function()
{

	var scripts =
	[
		"http://cdn.gamequeryjs.com/jquery.gamequery.js",
		"Libs/jsface",
		"Game"
	];

	require(scripts, function()
	{
		// Animations declaration:

		// The background:
		var background = new $.gQ.Animation({imageURL: "Content/Images/TestBack.png"});

		// Initialize the game:
		$("#playground").playground({height: PLAYGROUND_HEIGHT, width: PLAYGROUND_WIDTH, keyTracker: true});

		// Initialize the background
		$.playground()
			.addGroup("background", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT})
			.addSprite("background", {animation: background, width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});

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
			game.Start();
		});

	});

});

