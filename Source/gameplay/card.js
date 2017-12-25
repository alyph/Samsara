'use strict';

/* globals World Descriptor Feature
	AttributeManager SourcedModifier
	Trigger_PassiveOnSelf Trigger_OnStart
	ActiveEffect Effect_AttributeModifier 
	EffectExecutionContext, executeEffect
*/

var CardType = new Enum
(
	"entity",
	"trait",
	"tag",
);

class CardDescriptor
{
	constructor()
	{
		this.displayName = "NO NAME";
		this.smallPortrait = null;
		this.startingFeatures = [];
		this.triggers = [];
		//Trait = false // Only used for trait
	}
}


/* exported Card */
class Card
{
	constructor()
	{
		this.type = CardType.entity;
		this.template = null;
		this.desc = new CardDescriptor();

		// attribute cache (for both runtime cards, and static tag cards)
		this.attributeCache = null;

		// Runtime.
		this.world = null;
		this.host = null;
		this.features = null;
		this.runtimeAttributes = null;


		// TODO: attributes 
		//this.state = null;
	}

	[World.Symbol.start](world)
	{
		// if (this.static)
		// {
		// 	console.error(`The card ${this.displayName} is static cannot be instanced.`);
		// 	return;
		// }
		this.world = world;

		this.features = populateFeatures(this, this.desc.startingFeatures);

		let state = new CardState();
		state.world = world;

		this.state = state;
		this.resetAttributes();

		// trigger on start
		this.triggerOnStart(this.desc);
	}

	[World.Symbol.end](world)
	{
		if (this.state)
		{
			this.placeIn(null);

			// TODO: clean up the attachments, traits and other spawned cards.
		}
	}

	destroy()
	{
		if (this.state)
		{
			this.state.world.destroy(this);
		}
	}

	get unique()
	{
		if (this.type === CardType.tag || this.type === CardType.entity)
			return true;

		return this.type === CardType.tag || (this.type === CardType.trait && this.desc.uniqueTrait);
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

		let modifierEffects = this.desc.triggerEffects(Trigger_PassiveOnSelf, null, Effect_AttributeModifier);

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
		let effects = desc.triggerEffects(Trigger_OnStart, null, ActiveEffect);

		let context = new EffectExecutionContext();
		context.world = this.state.world;
		context.subject = this;

		for (let {effect} of effects)
		{
			executeEffect(effect, context);
		}
	}

	// return the attribute instance for the given attribute type.
	// Should it return a value if the card does not support the attribute?
	getAttribute(attr)
	{
		throw ("impl");
	}

	// safer version of getAttribute(attr).value
	getAttributeValue(attr)
	{
		throw ("impl");	
	}

	getTrigger(triggerType)
	{
		throw ("impl");
	}
}

class Feature
{
	constructor()	
	{
		this.card = null;
		this.sources = [];
	}
}


// class TraitInfo
// {
// 	constructor()
// 	{
// 		this.trait = null;
// 		this.template = null;
// 		this.derivedCount = 0;
// 	}
// }

class CardState
{
	constructor()
	{
		this.world = null;
		this.attributeManager = new AttributeManager();
		this.area = null;
		this.host = null;
		this.traits = new Map();
		this.attachments = [];
		this.activated = false;
		this.targeted = false;
	}
}

function populateFeatures(host, featureDescs)
{
	// let featureInfos = [];
	// insertFeatureInfo(featureInfos, featureDescs, false);

	let mergedFeatures = [];
	// for (let {card, params, inherited} of featureInfos)
	// {
	// 	mergeFeature(mergedFeatures, card, params, inherited);
	// }

	for (let [card, ...params] of featureDescs)
	{
		mergeFeature(host, mergedFeatures, card, params, null);
	}

	return mergedFeatures;
}

// function insertFeatureInfo(infos, featureDescs, inherited)
// {
// 	for (let [card, ...params] of featureDescs)
// 	{
// 		infos.push({ card, params, inherited });

// 		insertFeatureInfo(infos, card.inheritedFeatures, true);
// 	}
// }

function mergeFeature(host, mergedFeatures, card, params, source)
{
	let existingFeature = findExistingFeature(mergedFeatures, card);

	let featureCard = null;

	if (existingFeature)
	{
		featureCard = existingFeature.card;
		source = source || featureCard; // Self sourced.

		if (existingFeature.sources.indexOf(source) === -1)
			existingFeature.sources.push(source); // TODO: source may need include params info, so later can be separate out.

		if (card.Type === CardType.tag ||
			card.Type === CardType.entity)
		{
			// Only one exist, and all inherited features will not be added one more time.
			// So we are done here.
			return;
		}
		else if (card.type === CardType.trait)
		{			
			// If we get here, this must be a mergeable trait.

			// Need check if the trait is unique, if unique this basically works just like tag,
			// and then we are done.

			// TODO: merge attribute for traits
		}
	}
	else
	{
		// Create a new feature
		
		if (card.type === CardType.tag)
		{
			featureCard = card;
		}
		else if (!host || !host.world)
		{
			console.error(`Host is null or not instanced: %o. Likely because the host is a trait and trait is only supposed to have tags in its feature list.`, host);
			return;
		}
		else if (card.type === CardType.trait)
		{
			// Create instanced card, but borrow most of the data from the template.
			// Setup the template.
			featureCard = host.world.Spawn(Card); // NOTE: capital card, the class.
			featureCard.type = CardType.trait;
			featureCard.template = card;
			featureCard.desc = card.desc;

			// Now populate features using the cache from the template card.
			if (!card.features)
				card.features = populateFeatures(null, card.desc.startingFeatures);

			featureCard.features = card.features;
			featureCard.host = host;
		}
		else if (card.type === CardType.entity)
		{
			// Spawn using the template card. But has no connection with the template card.
			featureCard = host.world.Spawn(card);
			featureCard.host = host;
		}

		if (featureCard)
		{
			let feature = new Feature();
			feature.card = featureCard;
			feature.sources.push(source || featureCard);
			mergedFeatures.push(feature);
		}
	}

	// Merge in inherited features.
	// NOTE: this step may be skipped if there's already existing features. (see if (existingFeature) branch.)
	for (let [card, ...params] of card.inheritedFeatures)
	{
		mergeFeature(host, mergedFeatures, card, params, featureCard);
	}
}

function findExistingFeature(features, card)
{
	if (card.type === CardType.tag ||
		card.type === CardType.entity)
	{
		// tag always merged uniquely.
		// entity if same, means the same instance, merge uniquely
		for (let feature of features)
		{
			if (feature.card === card)
				return feature;
		}
	}
	else if (card.type === CardType.trait)
	{
		// TODO: Only mergeable trait should run the search.
		for (let feature of features)
		{
			if (feature.card.template === card)
				return feature;
		}
	}

	return null;
}



