var Goal = Class(WorldState,
{
	constructor : function(def, params, value)
	{
		Goal.$super.call(this, def, params, value);
	}
});

var GoalType = Class(WorldStateDefinition,
{
	constructor : function()
	{
		GoalType.$super.call(this);
	},

	instanceClass : Goal
});


