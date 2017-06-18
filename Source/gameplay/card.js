'use strict';

/* globals World Descriptor 
	AttributeManager SourcedModifier
	Trigger_PassiveOnSelf Trigger_OnStart
	ActiveEffect Effect_AttributeModifier 
	EffectExecutionContext
*/

/* exported Card */
class Card
{
	constructor()
	{
		this.displayName = "NO NAME";
		this.smallPortrait = null;
		this.desc = new Descriptor();
		this.state = null;
	}

	[World.Symbol.start](world)
	{
		let state = new CardState();
		state.world = world;

		this.state = state;
		this.resetAttributes();

		// trigger on start
		this.triggerOnStart(this.desc);
	}

	// TODO: need to have an end function,
	// so it can clean up the attachments, traits and other spawned cards.

	destroy()
	{
		if (this.state)
		{
			this.placeIn(null);	
			this.state.world.destroy(this);
		}
	}

	get attributeManager()
	{
		return this.state ? this.state.attributeManager : null;	
	}

	getArea()
	{
		return this.state ? this.state.area : null;
	}

	placeIn(area, temporarily = false)
	{
		if (!this.state)
			throw ("Cannot place non-intanced cards.");

		if (area !== this.state.area)
		{
			let prevArea = this.state.area;
			if (prevArea)
			{
				let idx = prevArea.cards.indexOf(this);
				if (idx >= 0)
					prevArea.cards.splice(idx, 1);
			}

			this.state.area = area;
			if (area)
			{
				area.cards.push(this);
			}
		}
	}

	returnToArea()
	{
		throw ("not implemented!");
	}

	isOnField()
	{
		let state = this.state;
		let field = state.world.field;
		return state.area === field.partyArea ||
			state.area === field.encounterArea;
	}

	attachTo(card)
	{
		if (!this.state)
			throw ("Cannot attach non-intanced cards.");

		if (!card.state)
			throw ("Cannot attach to non-intanced cards.");

		this.placeIn(null);

		let prevHost = this.state.host;
		if (prevHost)
		{
			let idx = prevHost.state.attachments.indexOf(this);
			if (idx >= 0)
				prevHost.state.attachments.splice(idx, 1);
		}

		this.state.host = card;
		if (card)
		{
			card.state.attachments.push(this);
		}
	}

	isUsableOnAction(action)
	{
		return false;
	}

	addDescriptor(descriptor)
	{
		if (this.desc.descriptors.indexOf(descriptor) < 0)
		{
			this.desc.descriptors.push(descriptor);
			this.desc.rebuildCache();
			this.resetAttributes();
			this.triggerOnStart(descriptor); // TODO: duplicated descriptor will have the effects triggered (bad?)
		}
	}

	resetAttributes()
	{
		let baseValues = this.desc.mergedAttrValues;

		let modifierEffects = this.desc.triggerEffects(Trigger_PassiveOnSelf, Effect_AttributeModifier);

		let modifiers = new Map();
		for (let effectInfo of modifierEffects)
		{
			let attr = effectInfo.effect.attribute;
			let modifier = effectInfo.effect.modifier;
			let attrMods = modifiers.get(attr);
			if (!attrMods)
			{
				attrMods = [];
				modifiers.set(attr, attrMods);
			}
			attrMods.push(new SourcedModifier(attr, modifier, this));
		}

		this.state.attributeManager.setImmutableAttributes(baseValues, modifiers);
	}

	triggerOnStart(desc)
	{
		let effects = desc.triggerEffects(Trigger_OnStart, ActiveEffect);

		let context = new EffectExecutionContext();
		context.world = this.state.world;
		context.subject = this;

		for (let {effect} of effects)
		{
			effect.execute(context);
		}
	}
}

class CardState
{
	constructor()
	{
		this.world = null;
		this.attributeManager = new AttributeManager();
		this.area = null;
		this.host = null;
		this.attachments = [];
		this.activated = false;
		this.targeted = false;
	}
}



