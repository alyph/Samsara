
/* exported Script */
var Func = {};
var Proc = {};

class VarRef
{
	constructor()
	{
		this.name = "";
	}
}

var Script = new (function(global)
{
	this.buildContext = (world, locals, scope) =>
	{
		let context = new ScriptContext();

		context.globals.set("world", world);
		context.globals.set("player", world.player);
		context.globals.set("field", world.field);

		for (let localVar in locals)
		{
			context.locals.set(localVar, locals[localVar]);
		}

		context.scope.push(...scope);
	};

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
			if (arg.constructor === VarRef)
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

	Proc.AttachTo = class AttachTo extends ScriptProcedure
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
			card.state.attachTo(target);

			return true;
		}
	};


})(this);



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

