'use strict';
/* globals NumericDieRoll */

/*exported Encounter*/
class Encounter extends Entity
{
	constructor()
	{
		super();
		this.location = null;
		this.entities = [];
	}

	setup()
	{
		let global = this.world.global;
		this.location = MathEx.randomItem(global.locations);

		let group = MathEx.randomItem(global.groups);
		if (group)
		{
			let npcs = group.spawn(this.world);
			this.entities = this.entities.concat(npcs);	
		}		
	}
}

/*exported GroupNpcConfig*/
class GroupNpcConfig
{
	constructor()
	{
		this.templateName = ""; // A NPC card.
		this.chance = 1.0;
		this.countRoll = new NumericDieRoll(1, 1);
	}
}

/*exported GroupTemplate*/
class GroupTemplate
{
	constructor()
	{
		this.npcs = [];
	}

	spawn(world)
	{
		let spawnedNpcs = [];
		for (let npcConfig of this.npcs)
		{
			if (npcConfig.templateName &&
				Math.random() < npcConfig.chance)
			{
				let num = this.countRoll.roll();
				for (let i = 0; i < num; i++)
				{
					let npc = world.spawn(npcConfig.templateName);
					spawnedNpcs.push(npc);
				}
			}
		}

		return spawnedNpcs;
	}
}