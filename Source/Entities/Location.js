
var Locale = Class(Entity, 
{
	constructor : function()
	{
		Locale.$super.call(this);
		this.connections = [];
		this.froms = [];
		this.within = null;
		this.contained = [];
	},

	beginPlay : function(game, world)
	{
		Locale.$superp.beginPlay.call(game, world);

		if (this.within !== null)
			this.within.contained.push(this);

		for (var i = this.connections.length - 1; i >= 0; i--) 
		{
			this.connections[i].Locale.froms.push(this);
		};
	},

	isInside : function(Locale)
	{
		for (var test = this; test !== null; test = test.within)
		{
			if (test === Locale)
				return true;
		}
		return false;
	},

	getPotentialTravelOrigins : function(party)
	{
		if (party.Locale === this)
		{
			return [];
		}
		else if (party.isInside(this))
		{
			for (var loc = party.Locale; loc !== null; loc = loc.within) 
			{
				if (loc.within === this)
					return [loc];
			};
			throw ("cannot find sub Locale when party is actually inside the current Locale...");
		}
		else if (this.within !== null)
		{
			// TODO: sometimes should go to connected other Locales or even sub Locales
			return [this.within];
		}
		else
		{
			// TODO: sometimes it makes sense to go inside a sub Locale that connects to other outside Locales...
			return this.froms;
		}
	}
});

var Region = Class(Entity,
{
	constructor : function()
	{
		Region.$super.call(this);
		this.areas = [];
	}
});

