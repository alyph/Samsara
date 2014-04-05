var World = Class(
{
	$statics:
	{
		WorldData:
		{
			entities : {}
		}
	},

	constructor : function(game)
	{
		this.game = game;
		this.entities = {};
		this.initialize();
	},

	initialize : function()
	{
		this.createEntities();

		World.WorldData = null;
	},

	createEntities : function()
	{
		var entitiesData = World.WorldData.entities;

		for (var name in entitiesData)
		{
			this.entities[name] = this.createEntity(name, entitiesData[name]);
		}

		for (var name in entitiesData)
		{
			this.extendEntityData(this.entities[name], entitiesData[name]);
		}

		for (var name in this.entities)
		{
			this.entities[name].beginPlay(this.game, this);
		}
	},

	createEntity : function(name, data)
	{
		var entity;
		if (typeof data.$base === "function")
		{
			entity = new data.$base();
		}
		else if (typeof data.$base === "string")
		{
			var def = Definition.get(data.$base);
			entity = Cloner.clone(def);
		}
		else
		{
			throw ("unsupported base types " + data.$base);
		}

		entity.$name = name;
		entity.$ref = true;
		return entity;
	},

	extendEntityData : function(entity, data)
	{
		Cloner.replaceReferences(data, this.entities);
		delete data["$base"];
		Cloner.extend(entity, data);
	},

	getEntity : function(name)
	{
		var entity = this.entities[name];
		if (entity === undefined)
			throw ("cannot find entity " + name);
		return entity;
	}
});

