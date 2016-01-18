'use strict';

class Keeper extends Entity
{
	constructor()
	{
		super();

		this.povs = [];
		this.focus = null;
	}

	beginPlay(game)
	{
		this.beginUpdate();
	}

	endPlay()
	{
	}

	update()
	{
		if (this.focus === null)
		{
			this.focus = MathEx.randomElementOfArray(this.povs);
			this.focus.chooseAction();
		}
	}

	handleAction(pov, action)
	{
		if (pov !== this.focus)
			throw ("wrong pov.");

		this.focus = null;

		// TODO: do action
		action.perform(pov);
	}

	activatePOV(pov)
	{
		if (this.povs.indexOf(pov) >= 0)
			throw ("pov already activated.");

		this.povs.push(pov);
	}
}