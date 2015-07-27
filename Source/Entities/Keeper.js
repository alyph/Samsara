var Keeper = Class(Entity,
{
	constructor : function()
	{
		Keeper.$super.call(this);

		this.povs = [];
		this.focus = null;
	},

	beginPlay : function(game)
	{
		this.beginUpdate();
	},

	endPlay : function()
	{
	},

	update : function()
	{
		if (this.focus == null)
		{
			this.focus = MathEx.randomElementOfArray(this.povs);
			this.focus.chooseAction();
		}
	},

	handleAction: function(pov, action)
	{
		if (pov !== this.focus)
			throw ("wrong pov.");

		this.focus = null;

		// TODO: do action
	},

	activatePOV : function(pov)
	{
		if (this.povs.indexOf(pov) >= 0)
			throw ("pov already activated.");

		this.povs.push(pov);
	}
});