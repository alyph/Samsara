var Activity = Class(WorldAction,
{
	constructor : function(def, params, context)
	{
		Activity.$super.call(this, def, params, context);
	},

	getDesc : function()
	{
		return "";
	},

	getTitle : function()
	{
		return this.def.title;
	},

	getImage : function()
	{
		return Sprites["cardCityGate"];
	}
});

var ActivityDefinition = Class(WorldActionDefinition,
{
	constructor : function()
	{
		ActivityDefinition.$super.call(this);
		this.title = "NO DESC ACTIVITY";
	},

	instanceClass : Activity
});

