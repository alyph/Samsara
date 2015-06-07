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

		if (this.isInstance)
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

	beginPlay : function()
	{
		this.world.keeper.activatePOV(this);
	},

	chooseAction : function()
	{
		this.scene.populatePOIs();
		this.controller.chooseAction(this);
	},

	actionChosen: function(action)
	{
		this.keeper.handleAction(this, action);
	}
});
