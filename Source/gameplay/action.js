'use strict';

/* globals Area buildScriptContext Card 
	AttributeManager, SourcedAttributeValue, AbilityValue
	EffectExecutionContext
*/

/* exported Action */
class Action
{
	constructor()
	{
		this.displayName = "NO NAME"; // May consider simply have a Card property to define everything within.
		this.smallPortrait = null;
		this.condition = null;
		this.naturalScore = 0;
		this.supportAbilities = [];
		this.counterAbilities = [];
		this.effect = null;
	}

	canPerform(world, instigator, target)
	{
		// If no condition then always performable.
		if (!this.condition)
			return true;

		// Build the script context
		let context = buildScriptContext(world, {instigator, target}, []);
		return this.condition.call(context);
	}
}

/* exported ActionInstance */
class ActionInstance
{
	constructor(action, world, instigator, target)
	{
		this.definition = action;
		this.world = world;
		this.instigator = instigator;
		this.target = target;
		this.addon = new Area();
	}

	spawnActionCard()
	{
		let name = this.definition.displayName.replace(' ',  '_');
		let card = this.world.spawn(Card, name);
		card.displayName = this.definition.displayName;
		card.smallPortrait = this.definition.smallPortrait;
		return card;
	}

	async execute(mods)
	{
		let def = this.definition;

		// Accumulate support abilities to form a starting bonus pool.
		let supportAttrMgr = new AttributeManager();
		let baseValues = new Map();

		for (let attr of def.supportAbilities)
		{
			let value = new SourcedAttributeValue(
				attr, this.instigator.attributeManager.getAttribute(attr), this.instigator);

			baseValues.set(attr, value);
		}
		supportAttrMgr.setImmutableAttributes(baseValues);


		// Accumulate counter abilities to form counter pool.
		let counterAttrMgr = new AttributeManager();
		baseValues = new Map();

		for (let attr of def.counterAbilities)
		{
			let value = new SourcedAttributeValue(
				attr, this.target.attributeManager.getAttribute(attr), this.target);

			baseValues.set(attr, value);			
		}
		counterAttrMgr.setImmutableAttributes(baseValues);

		// Add natural difficulties.

		// Run "used in action" trigger on all mod cards.

		// Run "react to action" trigger on all played cards.
		// Maybe need separate react to instigator and target actions.

		// Draw fate cards based on advantage points.

		// fate cards -> event cards.

		// Resolve all events cards.

		// Generate final results (support score - counter score + modifiers).

		let score = def.naturalScore;

		for (let {value} of supportAttrMgr.attributes())
		{
			if (value instanceof AbilityValue)
			{
				score += value.score;
			}
		}

		for (let {value} of counterAttrMgr.attributes())
		{
			if (value instanceof AbilityValue)
			{
				score -= value.score;
			}
		}

		score = Math.round(score);

		// Let action handle the result.
		if (def.effect)
		{
			let context = new EffectExecutionContext();
			context.subject = this.instigator;
			context.world = this.world;
			context.params.set("instigator", this.instigator);
			context.params.set("target", this.target);
			context.params.set("score", score);
			def.effect.execute(context);
		}
		else
		{
			console.error(`Action ${def.displayName} has no effect.`);
		}

		return true;

	}
}

