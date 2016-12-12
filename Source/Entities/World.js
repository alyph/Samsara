'use strict';

/*exported World*/
class World extends BaseObject
{
	/*
	$statics:
	{
		WorldData:
		{
			entities : {}
		}
	},
*/

	constructor()
	{
		super();
		this.game = null;
		this.entities = {};
		
		this.global = null;
		//this.keeper = null;
		this.field = null;
		

		this.updateQueue = [];
		this.isUpdating = false;
		// this.isInstance = true;
		//this.isWaiting = false;
		//this.updateCount = 0;
		//this.turn = 0;
	}

	beginPlay(game)
	{
		this.game = game;
		for (var name in this.entities)
		{
			this.entities[name].beginPlay();//(game);
		}
	}

	// create : function()
	// {
	// 	this.loadFromArchive();
	// },

	// loadFromArchive: function()
	// {
	// 	var entities = Archive.select(function(o){ return o.isA(Entity) && o.isInstance; });
	// 	var l = entities.length;

	// 	for (var i = 0; i < l; i++) 
	// 	{
	// 		var entity = entities[i];
	// 		this.entities[entity.$name] = entity;
	// 	};

	// 	for (var i = 0; i < l; i++) 
	// 	{
	// 		entities[i].enterWorld(this);
	// 	};

	// 	for (var i = 0; i < l; i++) 
	// 	{
	// 		entities[i].beginPlay();
	// 	};
	// },

/*
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
		//entity.$ref = true;
		return entity;
	},

	extendEntityData : function(entity, data)
	{
		Cloner.replaceReferences(data, this.entities);
		delete data["$base"];
		Cloner.extend(entity, data);
	},
*/
	getEntity(name)
	{
		var entity = this.entities[name];
		if (entity === undefined)
			throw ("cannot find entity " + name);
		return entity;
	}

	findEntity(name)
	{
		return this.entities[name] || null;
	}

	onEntityEntered(entity)
	{
		var name = entity.name();
		if (this.entities.hasOwnProperty(name))
			throw ("Entity with the same name already exists! " + name);

		this.entities[name] = entity;
	}

	spawn(base, props, name)
	{
		if (this.game === null)
			throw ("cannot spawn entities before game has begun play!");

		//props.isInstance = true;
		var entity = Archive.create(base, props, name);
		if (entity !== null)
		{
			//this.entities[name] = entity;
			//entity.enterWorld(this);
			entity.beginPlay();//(this.game);
		}

		return entity;

		// var entity = this.createEntity(name, data);
		// this.extendEntityData(entity, data);
		// this.entities[name] = entity;
		// entity.beginPlay(this.game, this);
		// return entity;
	}

	destroy(entity)
	{
		if (this.game === null)
			throw ("cannot destroy entities before game has begun play!");

		var name = entity.name();
		if (this.entities[name] !== entity)
			throw ("entity does not exist in the world, cannot be destroyed");

		entity.endPlay();
		//entity.leaveWorld();
		delete this.entities[name];
		Archive.delete(entity);
	}

	// run : function()
	// {
	// 	do
	// 	{
	// 		this.update();
	// 	} while(!this.isWaiting);
	// },

	addToUpdate(entity)
	{
		if (this.isUpdating)
			throw ("cannot modify update queue when updating");

		if (entity.world !== this)
			throw ("cannot update entity not in the world");

		this.updateQueue.push(entity);
	}

	removeFromUpdate(entity)
	{
		if (this.isUpdating)
			throw ("cannot modify update queue when updating");
		
		var idx = this.updateQueue.indexOf(entity);
		if (idx >= 0)
			this.updateQueue.splice(idx, 1);
	}

	update()
	{
		this.isUpdating = true;

		var l = this.updateQueue.length;
		for (var i = 0; i < l; i++) 
		{
			this.updateQueue[i].update();
		}

		this.isUpdating = false;

		// this.beginUpdate();

		// this.turn++;

		// var entry = this.requestUpdateEntry();
		// var queue = this.updateQueue;
		// this.updateCount = queue.length;

		// for (var i = queue.length - 1; i >= 0; i--) 
		// {
		// 	var entity = queue[i];
		// 	if (entity.$turn === this.turn)
		// 	{
		// 		throw ("the entity is updated twice!");
		// 		continue;
		// 	}

		// 	entity.$turn = this.turn;			
		// 	entry.update(entity);

		// 	if (!entry.isFinished())
		// 	{
		// 		entry = this.requestUpdateEntry();
		// 	}
		// };

		// if (this.updateCount === 0)
		// {
		// 	this.endUpdate();
		// }
		// else
		// {
		// 	this.isWaiting = true;
		// }
	}

	// beginUpdate : function()
	// {
	// 	this.isUpdating = true;
	// },

	// endUpdate : function()
	// {
	// 	this.isUpdating = false;
	// 	throw ("not implemented!");
	// },

	// requestUpdateEntry : function()
	// {
	// 	return new World.UpdateEntry(this);
	// },

	// updateEntryFinished : function(entry)
	// {
	// 	this.updateCount--;
	// 	throw ("not implemented!");

	// 	if (this.updateCount === 0 && this.isWaiting)
	// 	{
	// 		this.isWaiting = false;
	// 		this.endUpdate();
	// 		this.run();
	// 	}
	// }
}


// World.UpdateEntry = function()
// {
// 	var STATE_Instant = 0;
// 	var STATE_Updating = 1;
// 	var STATE_Finished = 2;

// 	return Class(
// 	{
// 		constructor : function(world)
// 		{
// 			this.world = world;
// 			this.entity = null;
// 			this.state = STATE_Instant;
// 		},

// 		isFinished : function()
// 		{
// 			return this.state === STATE_Instant || this.state === STATE_Finished;
// 		},

// 		update : function(entity)
// 		{
// 			this.entity = entity;
// 			this.state = STATE_Instant;

// 			entity.update(this);

// 			if (this.state === STATE_Instant)
// 				this.world.updateEntryFinished(this);
// 		},

// 		begin : function()
// 		{
// 			if (this.state !== STATE_Instant)
// 				throw ("cannot begin update, if it's already updating or finished!");

// 			this.state = STATE_Updating;
// 		},

// 		end : function()
// 		{
// 			if (this.state !== STATE_Updating)
// 				throw ("cannot end update, if it is not updating currently!");

// 			this.state = STATE_Finished;
// 			this.world.updateEntryFinished(this);
// 		}
// 	});
// }();


