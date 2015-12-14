var BaseObject = Class(
{
	constructor : function()
	{
		this.$name = "";
		this.$baseObj = null;
	},

	$canBeSubObject: true,

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
	},

	name: function()
	{
		return this.$name.substr(1);
	},

	isInstance : function()
	{
		return Archive.isInstance(this);
	},
});