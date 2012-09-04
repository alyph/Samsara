Core.Component("Map",
{

});

Core.Component("Explore",
{
	addAction : function(game, card, actions)
	{
		actions.push(
		{
			text : "Explore",
			handler : this._doExplore
		});
	},

	_doExplore : function(game, card)
	{
	}
});