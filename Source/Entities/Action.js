
var ActionDefinition = Class(BaseObject,
{
	constructor : function()
	{
		ActionDefinition.$super.call(this);

		this.predicate = null;
	},

	populateActions : function(pov, actions)
	{
		if (this.evaluate(pov))
		{
			this.instantiate(pov, actions);
		}
	},

	evaluate : function(pov)
	{
		if (this.predicate === null)
			return true;

		var keyValues =
		{
			pov: pov,
			scene: pov.scene
		};

		return this.predicate.eval(keyValues);
	},

	instantiate : function(pov, actions)
	{
		actions.push(new Action(this));
	}
});

var Action = Class(
{
	constructor : function(def)
	{
		this.def = def;
	}
});
