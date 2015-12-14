var PointOfView = Class(Entity,
{
	constructor : function()
	{
		PointOfView.$super.call(this);

		this.controller = null;
		this.characters = [];
		this.locale = null;
		this.scene = null;
	},

	init : function()
	{
		PointOfView.$superp.init.call(this);

		if (this.isInstance())
		{
			for (var i = this.characters.length - 1; i >= 0; i--) 
			{
				this.characters[i].pov = this;
			};
				
			if (this.scene !== null)
			{
				this.scene.pov = this;
			}
		}
	},

	beginPlay : function(game)
	{
		this.world.keeper.activatePOV(this);
	},

	populateActions : function()
	{
		var actions = [];
		var actionDefs = Archive.getAll(Global.ActionsNamespace);
		var numActionDefs = actionDefs.length;
		for (var i = 0; i < numActionDefs; i++) 
		{
			actionDefs[i].populateActions(this, actions);
		};
		return actions;
	},

	chooseAction : function()
	{
		//this.scene.populatePOIs();
		var actions = this.populateActions();
		this.controller.chooseAction(this, actions);
	},

	actionChosen: function(action)
	{
		this.keeper.handleAction(this, action);
	}
});
