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
