'use strict';

/* globals Script Area */

var ActionMode = new Enum
(
	"hand",
	"field",
	"attached"
);

var ActionTarget = new Enum
(
	"single",
	"party",
	"encounter"
);

/* exported Action */
class Action
{
	constructor()
	{
		this.mode = ActionMode.hand;
		this.target = ActionTarget.single;
		this.targetCondition = null;
		this.effect = null;
	}
}

/* exported ActionInstance */
class ActionInstance
{
	constructor(card, action)
	{
		this.card = card;
		this.definition = action;

		this.instigator = null;
		this.target = null;
		this.addon = new Area();
		//this.targetConditions = [];
	}

	isUsable()
	{
		if (!this.isInRightMode())
			return false;

		// TODO: check if there's at least one valid target.

		return true;
	}

	isInRightMode()
	{
		let area = this.card.state.area;
		let world = this.card.state.world;
		let player = world.player;

		switch (this.definition.mode)
		{
			case ActionMode.hand:
			{
				return area === player.hand;
			}
			case ActionMode.field:
			{
				return this.card.isOnField();
			}
			// TODO: other modes
		}

		return false;
	}

	isValidTarget(target)
	{
		// Make sure the target is on field.
		if (!target.isOnField())
			return false;

		// If no condition then any target is fine.
		let cond = this.definition.targetCondition;
		if (!cond)
			return true;

		// Build the script context
		let world = this.card.state.world;
		let card = this.card;
		let action = this;
		let context = Script.buildContext(world, {target, card, action}, [target]);

		return cond.call(context);
	}

	async execute(target)
	{		
		if (this.definition.target === ActionTarget.single && !target)
			return false;

		let effect = this.definition.effect;
		if (!effect)
			return false;

		// Build the script context
		let world = this.card.state.world;
		let card = this.card;
		let action = this;
		let context = Script.buildContext(world, {card, target, action}, [card]);

		return await effect.run(context);
	}
}

