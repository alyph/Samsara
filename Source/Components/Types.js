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

Core.Component("Company",
{
});

Core.Component("Story",
{
});

Core.Component("Site",
{
	postLoad : function(def)
	{
		if (def.location === undefined)
			return;

		if ((typeof def.location === 'string') && Core.hasCard(def.location))
			return;

		var locDef = $.extend({}, def, def.location);
		var name = "LocationOf" + this.name;
		this.location = name;
		Core.addCard(name, locDef);
	},

	addActivity : function(game, card, activites)
	{
		activites.push(card.definition.visiting);
	}
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