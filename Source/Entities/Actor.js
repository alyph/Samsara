
/*
var Actor = Class(Entity,
{
	$statics:
	{
		Types: { None: 0, PC: 1, NPC: 2, Prop: 3 }
	},

	constructor : function()
	{
		Actor.$super.call(this);

		this.type = Actor.Types.None;
		this.group = null;
		this.elements = null;
		this.actions = [];
		this.scene = null;
	},

	perform : function()
	{
		for (var i = this.actions.length - 1; i >= 0; i--)
		{
			var action = this.actions[i];
			if (!action.step())
			{
				this.actions.splice(i, 1);
			}
		}

		return this.actions.length > 0;		
	},

	enterScene : function(scene)
	{
		scene.addActor(this);
	},

	leaveScene : function()
	{
		this.scene.removeActor(this);
	}
});
*/