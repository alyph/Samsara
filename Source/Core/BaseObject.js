var BaseObject = Class(
{
	constructor : function()
	{
		this.$name = "";
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