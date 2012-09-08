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
		var encounter = card.instance.encounterDeck.drawEncounter();
		var encounterCard = game.table.placeInScene(encounter, card.slot);
		game.setActiveCard(encounterCard);
		card.destroy();
		game.nextTurn();
	}
});

Core.Component("Monster",
{

});