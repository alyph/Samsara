
class Attribute
{
	constructor()
	{
		this.displayName = "NAMELESS ATTRIBUTE";
		this.mutable = false;
		this.default = null;
	}

	modify(manager, value, modifiers)
	{
		console.error(`The attribute ${this.displayName} does not implement modify() function cannot be immutable.`);
	}

	mutate(value, operation, context)
	{
		console.error(`The attribute ${this.displayName} does not implement mutate() function cannot be mutable.`);
	}
}

class ValueReference
{
	constructor(raw, param = "", attr = null)
	{
		this.raw = raw;
		this.attribute = attr;
		this.param = param;
	}

	getValue(context)
	{
		if (this.attribute)
		{
			let manager = context.resolveEntityAttributeManager(this.param);
			return manager.getAttribute(this.attribute);
		}
		else if (this.param)
		{
			return context.resolveParameter(this.param);
		}
		else
		{
			return this.raw;
		}
	}
}

/* exported SourcedAttributeValue */
class SourcedAttributeValue
{
	constructor(attr, value, source)
	{
		this.attribute = attr;
		this.value = value;
		this.source = source;
	}
}

/* exported SourcedAttributeModifier */
class SourcedAttributeModifier
{
	constructor(attr, modifier, source)
	{
		this.attribute = attr;
		this.modifier = modifier;
		this.source = source;
	}
}

/* exported AttributeManager */
class AttributeManager
{
	constructor()
	{
		this.mutableAttrs = new Map();
		this.immutableAttrs = new Map();
		this.baseValues = null;
		this.modifiers = null;
	}

	setImmutableAttributes(baseValues, modifiers = null)
	{
		this.baseValues = baseValues;
		this.modifiers = modifiers;
		this.immutableAttrs.clear();
	}

	addModifiers(attr, modifiers)
	{
		if (this.modifiers && this.modifiers.has(attr))
		{
			let existing = this.modifiers.get(attr);
			this.modifiers.set(attr, existing.concat(modifiers));
		}
		else
		{
			this.modifiers.set(attr, modifiers);
		}
	}

	getAttribute(attr)
	{
		if (attr.mutable)
		{
			let value = this.mutableAttrs.get(attr);
			return (value !== undefined) ? value : attr.default;
		}
		else
		{
			let value = this.immutableAttrs.get(attr);
			if (value !== undefined)
			{
				return value;
			}
			else
			{
				value = this.cacheImmutableAttribute(attr);
				return (value !== undefined) ? value : attr.default;
			}
		}
	}

	*attributes()
	{
		for (let attribute of this.baseValues.keys())
		{
			let value = this.getAttribute(attribute);
			yield { attribute, value };
		}

		for (let { key, value } of this.mutableAttrs)
		{
			let attribute = key;
			yield { attribute, value };
		}
	}

	mutateAttribute(attr, operation, context)
	{
		if (!attr.mutable)
		{
			console.error(`The attribute ${this.displayName} is not mutable, cannot be mutated.`);
			return;
		}

		let value = attr.mutate(this.getAttribute(attr), operation, context);
		this.mutableAttrs.set(attr, value);
	}

	cacheImmutableAttribute(attr)
	{
		let sourcedBaseValue = (this.baseValues ? this.baseValues.get(attr) : null);
		let baseValue = (sourcedBaseValue ? sourcedBaseValue.value : attr.default);
		let finalValue = baseValue;
		let sourcedModifiers = (this.modifiers ? this.modifiers.get(attr) : null);
		if (sourcedModifiers)
		{
			let modifiers = [];
			for (let {modifier} of sourcedModifiers)
			{
				modifiers.push(modifier);
			}

			finalValue = attr.modify(this, baseValue, modifiers);
		}
		
		this.immutableAttrs.set(attr, finalValue);
		return finalValue;
	}
}



/* exported NumberAttribute */
class NumberAttribute extends Attribute
{
	constructor()
	{
		super();

		this.min = Number.NEGATIVE_INFINITY;
		this.max = Number.POSITIVE_INFINITY;
		this.default = 0;
	}

	modify(manager, value, modifiers)
	{
		let add = 0;
		let mul = 1;

		for (let mod of modifiers)
		{
			switch (mod.type)
			{
				case NumberModType.additive: add += mod.value; break;
				case NumberModType.multiplicative: mul *= mod.value; break;
			}
		}

		return MathEx.clamp((value + add) * mul, this.min, this.max);
	}
}

var NumberModType = new Enum("additive", "multiplicative");

/* exported NumberModifier */
class NumberModifier
{
	constructor()
	{
		this.type = NumberModType.additive;
		this.value = 0;
	}
}


/* exported AbilityAttribute */
class AbilityAttribute extends Attribute
{
	constructor()
	{
		super();

		this.min = new AbilityValue(0, 0);
		this.default = new AbilityValue(0, 0);
	}

	modify(manager, value, modifiers)
	{
		let add = new AbilityValue(0, 0);
		let mul = new AbilityValue(1, 1);

		for (let mod of modifiers)
		{
			switch (mod.type)
			{
				case AbilityModType.additive: add.addAssign(mod.value); break;
				case AbilityModType.multiplicative: mul.multiplyAssign(mod.value); break;
			}
		}

		let finalValue = value.clone();
		finalValue.addAssign(add);
		finalValue.multiplyAssign(mul);
		finalValue.scorePoints = Math.max(finalValue.scorePoints, this.min.scorePoints);
		finalValue.advantagePoints = Math.max(finalValue.advantagePoints, this.min.advantagePoints);
		
		return finalValue;
	}
}

class AbilityValue
{
	constructor(score = 0, advantage = 0)
	{
		this.score = score;
		this.advantage = advantage;
	}

	clone()
	{
		return new AbilityValue(this.score, this.advantage);
	}

	addAssign(other)
	{
		this.score += other.score;
		this.advantage += other.advantage;
	}

	multiplyAssign(other)
	{
		this.score *= other.score;
		this.advantage *= other.advantage;
	}
}

var AbilityModType = new Enum("additive", "multiplicative");

/* exported AbilityModifier */
class AbilityModifier
{
	constructor()
	{
		this.type = AbilityModType.additive;
		this.value = new AbilityValue(0, 0);
	}
}




/* exported TokenAttribute */
class TokenAttribute extends Attribute
{
	constructor()
	{
		super();

		this.mutable = true;

		this.min = new ValueReference(0);
		this.max = new ValueReference(Number.POSITIVE_INFINITY);
		this.default = 0;
	}

	mutate(value, operation, context)
	{
		let change = operation.value.getValue(context);
		let min = this.min.getValue(context);
		let max = this.max.getValue(context);
		let newVal = value;

		switch(operation.type)
		{
			case TokenOpType.set: 		newVal = (change); 			break;
			case TokenOpType.add: 		newVal = (value + change); 	break;
			case TokenOpType.remove: 	newVal = (value - change); 	break;
			case TokenOpType.min: 		newVal = (min); 			break;
			case TokenOpType.max: 		newVal = (max); 			break;				
		}

		return MathEx.clamp(newVal, min, max);
	}
}

var TokenOpType = new Enum("set", "add", "remove", "min", "max");

/* exported TokenOperation */
class TokenOperation
{
	constructor()
	{
		this.type = TokenOpType.set;
		this.value = new ValueReference(0);
	}
}
