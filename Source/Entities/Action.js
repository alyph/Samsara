/*global ActionDefinition: true*/
'use strict';

class ActionDefinition extends BaseObject
{
	constructor()
	{
		super();

		this.predicate = null;
		this.targetKey = "";
		this.customLabel = null;
	}

	populateActions(pov, actions)
	{
		if (this.evaluate(pov))
		{
			this.instantiate(pov, actions);
		}
	}

	evaluate(pov)
	{
		if (this.predicate === null)
			return true;

		var keyValues =
		{
			pov: pov,
			scene: pov.scene
		};

		return this.predicate.eval(keyValues);
	}

	instantiate(pov, actions)
	{
		var targetScenes = pov.scene.scenes[this.targetKey];
		if (targetScenes)
		{
			for (var i = 0; i < targetScenes.length; i++) 
			{
				actions.push(new Action(this, targetScenes[i]));
			}
		}
	}

	Label(target)
	{
		if (this.customLabel)
			return this.customLabel;

		return target.Label();
	}

	Portrait(target)
	{
		return target.Portrait();
	}

	perform(pov, target)
	{
		pov.scene = target;
	}
}

class Action
{
	constructor(def, target)
	{
		this.def = def;
		this.target = target; // HACK: target right now is a scene prototype?!
	}

	Label()
	{
		return this.def.Label(this.target);
	}

	Portrait()
	{
		return this.def.Portrait(this.target);
	}

	perform(pov)
	{
		this.def.perform(pov, this.target);
	}
}
