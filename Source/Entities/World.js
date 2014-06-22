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
		if (this.entities[name])
			throw ("duplicated entity with name " + name);

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
	},

	findEntity : function(name)
	{
		return this.entities[name] || null;
	},

	spawn : function(name, data)
	{
		var entity = this.createEntity(name, data);
		this.extendEntityData(entity, data);
		this.entities[name] = entity;
		entity.beginPlay(this.game, this);
		return entity;
	},

	destroy : function(entity)
	{
		if (this.entities[entity.$name] !== entity)
			throw ("entity does not exist in the world, cannot be destroyed");

		delete this.entities[entity.$name];
		entity.endPlay();
	},

	step : function(steps)
	{
		
	}
});

