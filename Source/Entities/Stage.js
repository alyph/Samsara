
/*
var StageDefinition = Class(
{
	constructor : function() {}
});
*/


var Stage = Class(
{
	constructor : function()
	{
		this.actors = [];
	}, 

	addActor : function(actor)
	{
		this.actors.push(actor);
	},

	removeActor : function(actor)
	{
		this.actors.splice(this.actors.indexOf(actor), 1);
	},

	getObjects : function()
	{
		var objects = [];

		for (var i = 0; i < this.actors.length; i++) 
		{
			var actor = this.actors[i];
			if (actor.type == Actor.Types.NPC || actor.type === Actor.Types.Prop)
			{
				objects.push(actor);
			}
		};

		return objects;
	}

/*	constructor : function(game, def)
	{
		this._game = game;
		//this._party = party;
		//this._site = site || null;
		this.def = def;
		this.entities = [];
	},

	populate : function()
	{
		//this.entities.push(this._game.companyDeck.draw());
	},

	addEntity : function(entity)
	{
		this.entities.push(entity);
	},

	scouting : function()
	{
		return "Nothing interesting here.";
	},

	detected : function()
	{
		return this.entities;
	}*/
});