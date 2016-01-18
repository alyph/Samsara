
var Activity = Class(WorldAction,
{
	constructor : function(def, params, context)
	{
		Activity.$super.call(this, def, params, context);
		this.seq = Cloner.clone(def.seq);
		this.context = Cloner.clone(context);
		this.params.attachTo(this.context);
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
		return Sprites["Explore"];
	},

	getSize: function()
	{
		return "";
	},

	step : function()
	{
		return this.seq.execute(this.context) !== Sequence.STATE_FINISHED;
	}
});

var ActivityDefinition = Class(WorldActionDefinition,
{
	constructor : function()
	{
		ActivityDefinition.$super.call(this);
		this.title = "NO DESC ACTIVITY";
		this.seq = null;
	},

	instanceClass : Activity
});

