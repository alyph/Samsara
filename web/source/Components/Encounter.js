
/*
Core.Component("Encounter",
{
	postLoad : function(def)
	{
		if (typeof def.story === 'string')
			return;

		var story = def.story;
		if (story._base === undefined)
			story._base = def._base === undefined ? StoryDefinition : def._base.story;

		if (story._base === undefined)
			throw ("Encounter story does not have proper base!");

		var name = "Story_Encounter" + this.name;
		this.story = name;
		Core.addDef(name, story);
	},

	beginPlay : function(card, game)
	{
		card.stage = new Stage(game, Core.getDef(card.definition.stage));

		// populate actors
		this._populateActors(card, game);
	},

	_populateActors : function(card, game)
	{
		card.actors = {};
		for (name in card.definition.actors)
		{
			var actorInfo = card.definition.actors[name];
			var actor = game.entityDeck.drawEntity(actorInfo.tags);
			card.actors[name] = actor;
			card.stage.addEntity(actor);
		}
	},

	_template :
	{
		startEncounter : function(party)
		{
			var params =
			{
				encounter : this,
				stage : this.stage,
				actors : this.actors
			};

			party.startStory(this.definition.story, params);
		}
	}

});*/