var BaseObject = Class(
{
	constructor : function()
	{
		this.$name = "";
		this.$baseObj = null;
	},

	init : function()
	{
	},

	isA: function(cls)
	{
		var current = this.$class;
		while (current)
		{
			if (current === cls)
				return true;

			current = current.$super;
		}
	}
});