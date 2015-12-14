/*global ActionDefinition: true*/

var ActionDefinition = Class(BaseObject,
{
	constructor : function()
	{
		ActionDefinition.$super.call(this);

		this.predicate = null;
		this.targetKey = "";
		this.customLabel = null;
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
		var targetScenes = pov.scene.scenes[this.targetKey];
		if (targetScenes)
		{
			for (var i = 0; i < targetScenes.length; i++) 
			{
				actions.push(new Action(this, targetScenes[i]));
			}
		}
	},

	Label : function(target)
	{
		if (this.customLabel)
			return this.customLabel;

		return target.Label();
	},

	Portrait : function(target)
	{
		return target.Portrait();
	}
});

var Action = Class(
{
	constructor : function(def, target)
	{
		this.def = def;
		this.target = target; // HACK: target right now is a scene prototype?!
	},

	Label : function()
	{
		return this.def.Label(this.target);
	},

	Portrait : function()
	{
		return this.def.Portrait(this.target);
	}
});
