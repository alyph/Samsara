'use strict';

class Trigger
{
	constructor()
	{
		this.effects = [];
	}

	matches(info)
	{
		return true;
	}
}

/* exported Trigger_PassiveOnSelf */
class Trigger_PassiveOnSelf extends Trigger
{
	constructor()
	{
		super();
	}
}

/* exported Trigger_OnStart */
class Trigger_OnStart extends Trigger
{
	constructor()
	{
		super();
	}
}

class Trigger_OnEvent extends Trigger
{
	constructor()
	{
		super();

		this.eventType = GameEvent;
		this.condition = null;
	}

	matches(info)
	{
		if (!info || !info.world || !info.event)
			return false;

		if (!(info.event instanceof this.eventType))
			return false;

		// Build the script context
		let context = buildScriptContext(info.world, info.event.params, []);
		if (!this.condition.call(context))
			return false;

		return true;
	}
}

class GameEvent
{
	constructor()
	{
		this.target = null;
		this.params = new Map();
	}
}

class GameEvent_AttributeMutated extends GameEvent
{
	constructor()
	{
		super();
		this.params.set("attribute", null);
		this.params.set("oldValue", undefined);
		this.params.set("newValue", undefined);
	}
}

