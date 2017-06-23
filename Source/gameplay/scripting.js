'use strict';

/* exported Script */
var Func = {};
var Proc = {};

class VariableReference
{
	constructor(name = "")
	{
		this.name = name;
	}
}

function VarRef(name)
{
	return new VariableReference(name);
}

class ScriptFunction
{
	constructor()
	{
	}

	call(context)
	{
		throw ("NOT IMPLEMENTED");
	}
}

class ScriptProcedure
{
	constructor()
	{
	}

	async run(context)
	{
		throw ("NOT IMPLEMENTED");
	}
}

class ScriptContext
{
	constructor()
	{
		this.globals = new Map();
		this.locals = new Map();
		this.scope = [];
	}

	resolvePreArgs(...args)
	{
		let values = [];
		values.length = args.length;
		for (let i = 0; i < args.length; i++)
		{
			let arg = args[i];
			if (arg === undefined && this.scope.length > i)
			{
				values[i] = this.scope[i];
				continue;
			}

			values[i] = this.resolveArg(arg);
		}
		return values;
	}

	resolvePostArgs(...args)
	{
		let values = [];
		values.length = args.length;
		for (let i = 0; i < args.length; i++)
		{
			let arg = args[i];
			values[i] = this.resolveArg(arg);
		}
		return values;
	}

	resolveArg(arg)
	{
		if (arg.constructor === VariableReference)
		{
			if (this.locals.has(arg.name))
			{
				return this.locals.get(arg.name);
			}
			else if (this.globals.has(arg.name))
			{
				return this.globals.get(arg.name);
			}
			else
			{
				console.error(`Failed to find the variable '${arg.name}'`);
				return undefined;
			}
		}
		else if (arg instanceof ScriptFunction)
		{
			return arg.call(this);
		}
		else
		{
			// Raw value.
			return arg;
		}
	}
}

function buildScriptContext(world, locals, scope)
{
	let context = new ScriptContext();

	context.globals.set("world", world);
	context.globals.set("player", world.player);
	//context.globals.set("field", world.field);

	if (locals)
	{
		context.locals = locals;
	}

	context.scope.push(...scope);
	return context;
}

Proc.AttachTo = class extends ScriptProcedure
{
	constructor()
	{
		super();
		this.card = undefined;
		this.target = undefined;
	}

	async run(context)
	{
		let [card] = context.resolvePreArgs(this.card);
		let [target] = context.resolvePostArgs(this.target);

		if (!card || !target)
			return false;

		// TODO play anim
		card.attachTo(target);

		return true;
	}
};

Func.HasDesc = class extends ScriptFunction
{
	constructor()
	{
		super();
		this.card = undefined;
		this.descs = [];
	}

	call(context)
	{
		let [card] = context.resolvePreArgs(this.card);
		if (!card) return false;

		return card.desc.hasAllDescriptors(...this.descs);
	}
};

Func.And = class extends ScriptFunction
{
	constructor()
	{
		super();
		this.conditions = [];
	}

	call(context)
	{
		for (let cond of this.conditions)
		{
			if (!cond.call(context))
				return false;
		}

		return true;
	}
};

var CompareOp = new Enum("equal", "less", "lessEqual", "greater", "greaterEqual");

Func.Compare = class extends ScriptFunction
{
	constructor()
	{
		super();
		this.op = CompareOp.equal;
		this.a = undefined;
		this.b = undefined;
	}

	call(context)
	{
		let [a] = context.resolvePreArgs(this.a);
		let [b] = context.resolvePostArgs(this.b);

		switch (this.op)
		{
			case CompareOp.equal: 			return (a === b);
			case CompareOp.less: 			return (a < b);
			case CompareOp.lessEqual: 		return (a <= b);
			case CompareOp.greater: 		return (a > b);
			case CompareOp.greaterEqual: 	return (a >= b);
			default: return false;			
		}
	}
};

// $predicate("$child is a $base", function(child, base)
// {
// 	if (typeof base === 'string')
// 		base = Archive.find(base);

// 	if (!base)
// 		return false;

// 	var test = child;
// 	while (test)
// 	{
// 		if (test === base)
// 			return true;

// 		test = test.$baseObj;
// 	}

// 	return false;
// });

