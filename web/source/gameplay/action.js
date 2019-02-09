'use strict';

/* globals Area buildScriptContext Card 
	AttributeManager, SourcedAttributeValue, AbilityValue
	EffectExecutionContext, executeEffect
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

		let locals = new Map();
		locals.set("instigator", instigator);
		locals.set("target", target);

		// Build the script context
		let context = buildScriptContext(world, locals, []);
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
			//def.effect.execute(context);
			executeEffect(def.effect, context);
		}
		else
		{
			console.error(`Action ${def.displayName} has no effect.`);
		}

		return true;

	}
}

class SkillCheck
{
	constructor()
	{
		this.supportSkills = [];
		this.counterSkills = [];
	}
}


class Effect_Move extends ActiveEffect
{
	constructor()
	{
		super();

		this.check = new SkillCheck();
		this.result = null;
	}

	execute(context, events)
	{
		let instigator = context.resolveParameter("instigator");
		let target = context.resolveParameter("target");

		if (!instigator || !target)
		{
			console.error(`Missing instigator or target`);
			return;
		}


		// collect all the skills involved in the check (make a copy, since we will modify them at move time)
		let supportAttrs = collectSkillAttributes(instigator, this.check.supportSkills);
		let counterAttrs = collectSkillAttributes(target, this.check.counterSkills);


		// apply move time attribute modifiers from instigator/target's features to the collected attributes

		// apply other move time modifiers (like nullify certain types of attributes, or doulbe certain attributes)

		// calculate final score
		let supportScore = calcScore(supportAttrs);
		let counterScore = calcScore(counterAttrs);

		let finalScore = Math.round(Math.max(supportScore - counterScore, 0));

		if (!this.result)
		{
			console.error(`Move effect missing result.`);
			return;
		}

		// execute result effect with { action, instigator, target, score }
		let resultContext = new EffectExecutionContext();
		resultContext.subject = instigator;
		resultContext.world = context.world;
		resultContext.params.set("instigator", instigator);
		resultContext.params.set("target", target);
		resultContext.params.set("score", finalScore);
		executeEffect(this.result, resultContext);

		function collectSkillAttributes(card, skills)
		{
			let attrs = [];
			for (let skill of skills)
			{
				let attr = instigator.getAttribute(skill);
				if (attr)
				{
					attrs.push(attr.clone());
				}
				else
				{
					console.error(`Missing attribute ${skill.displayName} on card %o`, card);
				}
			}

			return attrs;
		}

		function calcScore(attributes)
		{
			let score = 0;
			for (let attr of attributes)
			{
				score += attr.value;
			}
			return score;
		}
	}
}
