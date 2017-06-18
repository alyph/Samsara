'use strict';

/* globals SourcedAttributeValue */

/* exported Descriptor */
class Descriptor
{
	constructor()
	{
		this.displayName = "";
		this.descriptors = [];
		this.attributes = new Map();
		this.triggers = [];
		this.cache = null;
	}

	get flattenedDescs()
	{
		this.buildCache();
		return this.cache.flattenedDescs;
	}

	get mergedAttrValues()
	{
		this.buildCache();
		return this.cache.mergedAttrValues;
	}

	hasAllDescriptors(...descriptors)
	{
		let myDescs = this.flattenedDescs;
		for (let desc of descriptors)
		{
			if (!myDescs.has(desc))
				return false;
		}
		return true;
	}

	triggerEffects(triggerType, effectType = null)
	{
		let effects = [];
		for (let desc of this.flattenedDescs)
		{
			for (let trigger of desc.triggers)
			{
				// TODO: test conditions
				if (trigger instanceof triggerType)
				{
					for (let effect of trigger.effects)
					{
						if (!effectType || (effect instanceof effectType))
						{
							effects.push(new TriggeredEffect(effect, trigger, desc));
						}
					}
				}
			}
		}
		return effects;
	}

	buildCache()
	{
		if (!this.cache) 
			this.cache = new DescriptorCache(this);
	}

	rebuildCache()
	{
		this.cache = new DescriptorCache(this);	
	}
}

class DescriptorCache
{
	constructor(descriptor)
	{
		this.flattenedDescs = flattenDescs(descriptor);
		this.mergedAttrValues = mergeAttrValues(descriptor);
	}
}

function flattenDescs(descriptor)
{
	let descList = [];
	for (let desc of descriptor.descriptors)
	{
		descList = descList.concat(Array.from(desc.flattenedDescs));
	}
	descList.push(descriptor);

	return new Set(descList);
}

function mergeAttrValues(descriptor)
{
	let mergedAttrs = new Map();
	for (let desc of descriptor.descriptors)
	{
		for (let [attr, sourcedVal] of desc.mergedAttrValues)
		{
			if (mergedAttrs.has(attr))
			{
				let existingVal = mergedAttrs.get(attr);
				if (existingVal.source !== sourcedVal.source)
				{
					console.error(`Multiple descriptors are setting the same attribute '${attr.displayName}': ${existingVal.source.displayName}: ${existingVal.value}, ${sourcedVal.source.displayName}: ${sourcedVal.value}. The latter will overwrite the former.`);
				}
			}

			mergedAttrs.set(attr, sourcedVal);
		}
	}

	for (let [attr, value] of descriptor.attributes)
	{
		// Can always overwrite super desc's attribute values without error.
		mergedAttrs.set(attr, new SourcedAttributeValue(attr, value, descriptor));
	}

	return mergedAttrs;
}

class TriggeredEffect
{
	constructor(effect = null, trigger = null, descriptor = null)
	{
		this.effect = effect;
		this.trigger = trigger;
		this.descriptor = descriptor;		
	}
}