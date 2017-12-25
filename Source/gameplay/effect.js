

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

	execute(context, events) { throw "not implemented"; }
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

	// TODO: no longer needed?
	resolveEntityAttributeManager(entityName)
	{
		// TODO: real impl and error msg.
		let entity = this.resolveEntity(entityName);
		return entity ? entity.attributeManager : null;
	}
}


function executeEffect(effect, context)
{
	let pendingQueue = [[effect, context]];
	let eventQueue = [];
	let world = context.world;

	while (pendingQueue.length > 0)
	{
		for (let [currentEffect, currentContext] of pendingQueue)
		{
			currentEffect.execute(currentContext, eventQueue);
		}

		pendingQueue.length = 0;

		for (let event of eventQueue)
		{
			let target = event.target;
			if (target)
			{
				let effects = target.desc.triggerEffects(Trigger_OnEvent, {world, event}, ActiveEffect);

				let eventContext = new EffectExecutionContext();
				eventContext.world = context.world;
				eventContext.subject = target;
				eventContext.params = event.params;

				for (let triggeredEffect of effects)
				{
					pendingQueue.push([triggeredEffect.effect, eventContext]);
				}
			}
			else
			{
				console.error(`Dispatched event ${event.constructor} with no target.`);
			}
		}
		eventQueue.length = 0;
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

	execute(context, events)
	{
		if (!this.attribute || !this.operation)
		{
			console.error(`Effect_MutateAttribute missing attribute or operation.`);
			return;
		}

		let target = context.resolveEntity(this.target);
		if (target && target.attributeManager)
		{
			let targetAttrs = target.attributeManager;
			let oldValue = targetAttrs.getAttribute(this.attribute);
			let newValue = targetAttrs.mutateAttribute(this.attribute, this.operation, context);

			let event = new GameEvent_AttributeMutated();
			event.target = target;
			event.params.set("attribute", this.attribute);
			event.params.set("oldValue", oldValue);
			event.params.set("newValue", newValue);
			events.push(event);
		}
	}
}

class Effect_Destroy extends ActiveEffect
{
	constructor()
	{
		super();
		this.target = "";
	}

	execute(context, events)
	{
		let target = context.resolveEntity(this.target);
		if (target)
		{
			context.world.destroy(target);
		}
	}	
}


