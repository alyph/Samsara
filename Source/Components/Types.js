Core.Component("Map",
{

});

Core.Component("City",
{

});

Core.Component("Follower",
{

});

Core.Component("Item",
{

});

Core.Component("Ability",
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
	addAction : function(game, card, actions)
	{
		actions.push(
		{
			text : "Attack",
			handler : this._doAttack
		});
	},

	_doAttack : function(game, card)
	{
	}
});

Core.Component("Story",
{
});

Core.Component("Location",
{
});


Core.Component("Quest",
{
});

Core.Component("Exploration", "Quest",
{
});

Core.Component("Encounter",
{
});