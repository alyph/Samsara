

class Effect
{
	constructor()
	{

	}
}

class PassiveEffect extends Effect
{
	constructor()
	{
		super();
	}
}

class ActiveEffect extends Effect
{
	constructor()
	{
		super();
	}

	execute(context) { throw "not implemented"; }
}

/* exported EffectExecutionContext */
class EffectExecutionContext
{
	constructor()
	{
		this.world = null;
		this.subject = null;
		this.params = new Map();
	}

	resolveEntity(entityName)
	{
		// TODO: real impl.
		return entityName ? this.resolveParameter(entityName) : this.subject;
	}

	resolveParameter(paramName)
	{
		let value = this.params.get(paramName);
		if (value === undefined)
		{
			console.error(`The context does not contain requested parameter '${paramName}'.`);
		}
		return value;
	}

	resolveEntityAttributeManager(entityName)
	{
		// TODO: real impl and error msg.
		let entity = this.resolveEntity(entityName);
		return entity ? entity.attributeManager : null;
	}
}

// what's in the context?
// should have one primary subject (self?), this is different per each trigger
// in this subject, it should have two properties
// 1. the primary owner (the card?)
// 2. an object that allow to access descriptors and attributes (add, remove desc and attr etc.) <- what should it be called? <- actor?
// the world maybe needed
// then a set of parameters, some of the parameters may be other subjects (like target, instigator etc.)

/* exported Effect_AttributeModifier */
class Effect_AttributeModifier extends PassiveEffect
{
	constructor()
	{
		super();
		this.attribute = null;
		this.modifier = null;
	}
}

/* exported Effect_MutateAttribute */
class Effect_MutateAttribute extends ActiveEffect
{
	constructor()
	{
		super();

		this.target = "";
		this.attribute = null;
		this.operation = null; // TODO: maybe we should figure out the operation based on the attribute type?
	}

	execute(context)
	{
		if (!this.attribute || !this.operation)
		{
			console.error(`Effect_MutateAttribute missing attribute or operation.`);
			return;
		}

		let targetAttrs = context.resolveEntityAttributeManager(this.target);
		if (targetAttrs)
		{
			targetAttrs.mutateAttribute(this.attribute, this.operation, context);
		}
	}
}