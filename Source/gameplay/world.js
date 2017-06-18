'use strict';

/*exported World*/
class World
{
	constructor()
	{
		this.game = null;
		//this.entities = {};
		
		this.rule = null;
		this.global = null;
		//this.keeper = null;

		this.field = null;
		this.player = null;
		
		this.updateQueue = [];
		this.isUpdating = false;
	}

	start(game)
	{
		this.game = game;

		let instances = [...Archive.instances];
		for (let inst of instances)
		{
			this.startInstance(inst);
		}
	}

	spawn(base, name)
	{
		let inst = Archive.create(base, name);
		if (inst)
		{
			this.startInstance(inst);
		}
		return inst;
	}

	startInstance(inst)
	{
		let start = inst[World.Symbol.start];
		if (start)
		{
			start.call(inst, this);
		}
	}

	destroy(inst)
	{
		Archive.destroy(inst);
	}
}

World.Symbol =
{
	start: Symbol("worldStart"),
};



