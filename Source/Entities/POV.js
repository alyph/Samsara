'use strict';

class PointOfView extends Entity
{
	constructor()
	{
		super();

		this.controller = null;
		this.characters = [];
		this.locale = null;
		this.scene = null;
		this.execIdx = 0;
	}

	init()
	{
		super.init();

		if (this.isInstance())
		{
			for (var i = this.characters.length - 1; i >= 0; i--) 
			{
				this.characters[i].pov = this;
			}
				
			if (this.scene !== null)
			{
				this.scene.pov = this;
			}
		}
	}

	beginPlay(game)
	{
		this.world.keeper.activatePOV(this);
	}

	act()
	{
		while (this.execute()) {}
	}

	execute()
	{
		if (this.execIdx >= this.scene.script.length)
		{
			this.execIdx = 0;
			this.chooseAction();
			return false;
		}
		else
		{
			let instruction = this.scene.script[this.execIdx++];
			return this["exec"+instruction.type](instruction);
		}
	}

	execRandomScene(instr)
	{
		var sceneDef = MathEx.randomElementOfArray(instr.scenes);
		if (!sceneDef)
			throw ("fail to find a random scene def.");

		var name = sceneDef.name() + "_inst";
		var old = Archive.find(name);
		if (old !== null)
			Archive.delete(old);

		var scene = Archive.create(name, { $base: sceneDef.name() });
		scene.pov = this;
		this.scene = scene;

		return true;
	}

	populateActions()
	{
		var actions = [];
		var actionDefs = Archive.getAll(Global.ActionsNamespace);
		var numActionDefs = actionDefs.length;
		for (var i = 0; i < numActionDefs; i++) 
		{
			actionDefs[i].populateActions(this, actions);
		}
		return actions;
	}

	chooseAction()
	{
		//this.scene.populatePOIs();
		var actions = this.populateActions();
		this.controller.chooseAction(this, actions);
	}

	actionChosen(action)
	{
		this.world.keeper.handleAction(this, action);
	}
}
