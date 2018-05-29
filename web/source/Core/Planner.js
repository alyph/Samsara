

var Planner = new (function()
{

	var ACTION_RESULT_STATUS_QUO = 0;
	var ACTION_RESULT_SUCCESSFUL = 1;
	var ACTION_RESULT_FAILED = 2;


	this.State = Class(
	{
		constructor : function()
		{
			this.value = null;
		},

		relates : function(state)
		{
			throw ("must be overridden");
		},

		implies : function(state)
		{
			return this.relates(state) && this.value.implies(state.value);
		},

		contradicts : function(state)
		{
			return this.relates(state) && this.value.contradicts(state.value);
		},

		eval : function(context)
		{
			throw ("must be overridden");	
		},

		findEffectiveActions : function(context)
		{
			throw ("must be overridden");
		},

		keyStr : function()
		{
			throw ("must be overridden");
		}
	});

	this.Action = Class(
	{
		constructor : function()
		{
			this.cost = 1;
			this.effects = [];
			this.preconditions = [];
		},

		result : function(state, context)
		{
			for (var i = this.preconditions.length - 1; i >= 0; i--) 
			{
				if (this.preconditions[i].contradicts(state))
					return ACTION_RESULT_FAILED;
			};

			var sufficient = false;

			for (var i = this.effects.length - 1; i >= 0; i--) 
			{
				var effect = this.effects[i];
				if (effect.relates(state))
				{
					if (effect.value.implies(state.value))
						sufficient = true;
					else
						return ACTION_RESULT_FAILED;
				}
			};

			return sufficient ? ACTION_RESULT_SUCCESSFUL : ACTION_RESULT_STATUS_QUO;
		}
	});

	var Node = Class(
	{
		constructor : function(goal)
		{
			this.states = goal;
			this.previous = null;
			this.action = null;
			this.cost = 0;
			this.evaled = false;
		},

		eval : function(context)
		{
			for (var i = this.states.length - 1; i >= 0; i--) 
			{
				if (!this.states[i].eval(context))
					return false;
			};

			return true;
		},

		search : function(context)
		{
			var actions = [];

			for (var i = this.states.length - 1; i >= 0; i--) 
			{
				Array.prototype.push.apply(actions, this.states[i].findEffectiveActions(context));
			};

			return actions;
		},

		next : function(action, context)
		{
			var states = [];

			var achieves = false;
			for (var i = 0; i < this.states.length; i++) 
			{
				var state = this.states[i];
				var result = action.result(state, context);
				if (result === ACTION_RESULT_FAILED)
				{
					return null;
				}
				else if (result === ACTION_RESULT_STATUS_QUO)
				{
					states.push(state);
				}
				else
				{
					achieves = true;
				}
			};

			if (!achieves)
				return null;

			merge(states, action.preconditions);
			return states;
		}
	});

	function merge(target, source)
	{
		var merged = {};
		for (var i = target.length - 1; i >= 0; i--) 
		{
			for (var j = source.length - 1; j >= 0; j--) 
			{
				if (target[i].relates(source[j]))
				{
					target.value.merge(source[j].value);
					merged[j] = true;
				}
			};
		};

		for (var i = source.length - 1; i >= 0; i--) 
		{
			if (merged[i])
				continue;

			target.push(source[i]);
		};
	}

	var OpenList = Class(
	{
		constructor : function()
		{
			this.nodes = [];
		},

		push : function(node)
		{
			for (var i = this.nodes.length - 1; i >= 0; i--)
			{
				if (node.cost < this.nodes[i].cost)
				{
					this.nodes.splice(i+1, 0, node);
					return;
				}
			}

			this.nodes.splice(0, 0, node);
		},

		pop : function(node)
		{
			return this.nodes.pop();
		},

		/*
		remove : function(node)
		{

		},*/

		isEmpty : function()
		{
			return this.nodes.length == 0;
		}
	});

	var ClosedList = Class(
	{
		constructor : function()
		{
			this.stateMapping = {};
			this.nodes = [];
		},

		push : function(node)
		{
			var idx = this.nodes.length;
			this.nodes.push(node);

			for (var i = node.states.length - 1; i >= 0; i--) 
			{
				var state = node.states[i];
				var key = "$" + state.keyStr();
				var value = "$" + state.value.toString();

				if (!this.stateMapping.hasOwnProperty(key))
					this.stateMapping[key] = {};

				var valueMapping = this.stateMapping[key];

				if (!valueMapping.hasOwnProperty(value))
				{
					valueMapping[value] = { _size : 0 };
				}

				var nodeSet = valueMapping[value];

				nodeSet[idx] = true;
				nodeSet._size++;
			};
		},

		find : function(states)
		{
			if (states.length === 0)
				throw ("empty states?");

			var sets = [];
			var smallest = -1;
			var smallestSize = -1;

			for (var i = states.length - 1; i >= 0; i--) 
			{
				var state = states[i];
				var key = "$" + state.keyStr();
				var value = "$" + state.value.toString();

				var valueMapping = this.stateMapping[key];
				if (valueMapping === undefined)
					return null;

				var set = valueMapping[value];
				if (set === undefined)
					return null;

				if (smallest < 0 || set._size < smallestSize)
				{
					smallest = i;
					smallestSize = set._size;
				}

				sets.push(set);
			};

			var smallestSet = sets[smallest];
			sets.splice(smallest, 1);

			var candidates = [];
			for (var idx in smallestSet)
			{
				if (idx !== '_size')
					candidates.push(idx);	
			}

			for (var setIdx = sets.length - 1; setIdx >= 0; setIdx--)
			{
				var set = sets[setIdx];
				for (var i = candidates.length - 1; i >= 0; i--)
				{
					if (!set.hasOwnProperty(candidates[i]))
						candidates.splice(i, 1);
				};
			};

			for (var i = 0; i < candidates.length; i++) 
			{
				var node = this.nodes[candidates[i]];
				if (node.states.length === states.length)
					return node;
			};

			return null;
		}
	});

	this.plan = function(goal, context)
	{
		if (!isArray(goal))
			throw ("goal must be a list of states! " + goal);

		if (context === undefined)
			throw ("must provide context");

		var start = new Node(goal);

		var open = new OpenList();
		open.push(start);

		var close = new ClosedList();
		close.push(start);

		while (!open.isEmpty())
		{
			var current = open.pop();
			if (current.evaled)
				continue;

			current.evaled = true;

			if (current.eval(context))
				return current;

			var actions = current.search(context);
			for (var i = 0; i < actions.length; i++) 
			{
				var action = actions[i];
				var states = current.next(action, context);
				var cost = current.cost + action.cost;

				var visited = close.find(states);

				var next = null;
				if (visited === null)
				{
					next = new Node(states);
					close.push(next);
				}
				else if (!visited.evaled && visited.cost > cost)
				{
					//open.remove(visited);
					next = visited;
				}
				else
				{
					continue;
				}

				next.previous = current;
				next.action = action;
				next.cost = cost;
				open.push(next);
			};
		};

		return null;
	};
})();


StateValue = Class(
{
	$statics:
	{
		parse : function(expression, context)
		{
			var stateValue = parseInternal(expression, context);
			if (context === undefined && (stateValue.exact === '?' || stateValue.min === '?' || stateValue.max === '?'))
				return undefined;

			return stateValue;

			function parseInternal(expression, context)
			{
				var e = expression.length - 1;

				if (expression[0] === '<')
				{
					return new StateValue(undefined, undefined, parseExcluded(expression.substr(1), false, context));
				}
				else if (expression[0] === '<=')
				{
					return new StateValue(undefined, undefined, parseIncluded(expression.substr(1), context))
				}
				else if (expression[0] === '>')
				{
					return new StateValue(undefined, parseExcluded(expression.substr(1), true, context), undefined)
				}
				else if (expression[0] === '>=')
				{
					return new StateValue(undefined, parseIncluded(expression.substr(1), context), undefined)
				}
				else if (expression[0] === '(' || expression[0] === '[' || expression[e] === ')' || expression[e] === ']')
				{
					if (!(expression[0] === '(' || expression[0] === '[') || !(expression[e] === ')' || expression[e] === ']'))
						throw ("interval not defined properly! " + expression);

					var tokens = expression.substring(1, expression.length - 1).split(/\s*,\s*/);
					if (tokens.length !== 2)
						throw ("interval must have 2 ends! " + expression);

					var min = expression[0] === '(' ? parseExcluded(tokens[0], true, context) : parseIncluded(tokens[0], context);
					var max = expression[e]	=== ')' ? parseExcluded(tokens[1], false, context) : parseIncluded(tokens[1], context);

					return new StateValue(undefined, min, max);
				}
				else
				{
					return new StateValue(parseIncluded(expression, context));
				}
			}

			function parseIncluded(expression, context)
			{
				if (context === undefined)
				{
					var value;
					try
					{
						value = JSON.parse(expression);						
					}
					catch (e) 
					{
						return '?';						
					}

					if (typeof value === 'number' || typeof value === 'boolean' || value === null)
						return value;
					else
						throw ("unsupported simple value: " + expression + " -> " + value);
				}
				else
				{
					return Evaluator.eval(expression, context);
				}
			}

			function parseExcluded(expression, greater, context)
			{
				if (value === '?')
					return value;

				value = parseIncluded(expression, context);

				if (typeof value !== 'number')
					throw ("min/max values must be number! " + expression + " -> " + value)

				return value + (greater ? 0.001 : -0.001);
			}		
		}
	},

	constructor : function(exact, min, max)
	{
		this.exact = exact;
		this.min = min;
		this.max = max;
	},

	eval : function(value)
	{
		if (this.exact !== undefined)
		{
			return this.exact === value;
		}
		else
		{
			if (this.min !== undefined && !(this.min <= value))
				return false;

			if (this.max !== undefined && !(this.max >= value))
				return false;

			return true;
		}
	},

	implies : function(value)
	{
		if (this.exact !== undefined)
		{
			return value.eval(this.exact);
		}
		else
		{
			if (value.exact !== undefined)
				return false;

			if (value.min !== undefined && !(value.min <= this.min))
				return false;

			if (value.max !== undefined && !(value.max >= this.max))
				return false;

			return true;
		}
	},

	contradicts : function(value)
	{
		if (this.exact !== undefined)
		{
			return !value.eval(this.exact);
		}
		else if (value.exact != undefined)
		{
			return !this.eval(value.exact);
		}
		else
		{
			if (this.min !== undefined && value.max !== undefined && !(this.min <= value.max))
				return true;

			if (this.max !== undefined && value.min !== undefined && !(this.max >= value.min))
				return true;

			return false;
		}
	},

	merge : function(value)
	{
		if (this.contradicts(value))
			throw ("not mergeable, two value contradit each other! " + "(" + this.exact + ", " + this.min + ", " + this.max + ")"+ "(" + value.exact + ", " + value.min + ", " + value.max + ")");

		if (this.exact !== undefined)
		{
			return;
		}
		else if (value.exact !== undefined)
		{
			this.exact = value.exact;
		}
		else
		{
			if (value.min !== undefined && (this.min === undefined || value.min > this.min))
				this.min = value.min;

			if (value.max !== undefined && (this.max === undefined || value.max < this.max))
				this.max = value.max;
		}
	},

	toString : function()
	{
		if (this.exact !== undefined)
		{
			return this.exact.toString();
		}
		else if (this.min !== undefined && this.max === undefined)
		{
			return ">=" + this.min;
		}
		else if (this.min === undefined && this.max !== undefined)
		{
			return "<=" + this.max;
		}
		else if (this.min !== undefined && this.max != undefined)
		{
			return this.min + "~" + this.max;
		}
		else
		{
			throw ("invalid state value");
		}
	}
});

WorldStateDefinition = Class(
{
	$statics:
	{
		ActionInfo : Class(
		{
			constructor : function()
			{
				this.action = null;
				this.params = [];
			}
		})
	},

	constructor : function()
	{
		this.params = null;
		this.expression = "";
		this.effectiveActions = [];
	},

	instanceClass : null,

	instantiate : function(params, value)
	{
		return new this.instanceClass(this, params, value);
	},

	addEffectiveAction : function(actionInfo)
	{
		this.effectiveActions.push(actionInfo);
	}
});

WorldState = Class(Planner.State,
{
	constructor : function(def, params, value)
	{
		WorldState.$super.call(this);
		this.def = def;
		this.params = new ConstParams(def.params, params);
		this.value = value;
	},

	relates : function(state)
	{
		return this.def === state.def && this.params.equals(state.params);
	},

	eval : function(context)
	{
		this.params.attachTo(context);
		var result = this.value.eval(Evaluator.eval(this.def.expression, context));
		this.params.detachFrom(context);
		return result;
	},

	keyStr : function()
	{
		var key = this.def.name();
		if (this.params.length > 0)
			key += "(" + this.params.toString() + ")";

		return key;
	},

	findEffectiveActions : function(context)
	{
		var actions = [];
		var effectiveActions = this.def.effectiveActions;
		for (var i = effectiveActions.length - 1; i >= 0; i--) 
		{
			var actionInfo = effectiveActions[i];
			var permutations = this.findPermutations(actionInfo.params, context);

			for (var p = permutations.length - 1; p >= 0; p--) 
			{
				var action = actionInfo.action.instantiate(permutations[p], context);//new actionInfo.class(actionInfo.def);
				//action.params.batch(permutations[p]);
				actions.push(action);
			};

		};

		return actions;
	},

	findPermutations : function(tokens, context)
	{
		permutations = [[]];
		this.params.attachTo(context);
		for (var i = 0; i < tokens.length; i++) 
		{
			var params = Evaluator.eval(tokens[i], context);
			params = isArray(params) ? params : [params];

			var l = permutations.length;
			for (var p = 0; p < l; p++) 
			{
				var permu = permutations.shift();

				for (var n = 0; n < params.length; n++)
				{
					permutations.push(permu.concat(params[n]));
				};
			};

		};
		this.params.detachFrom(context);
		return permutations;		
	}
});

WorldActionDefinition = Class(
{
	$statics:
	{
		StateInfo : Class(
		{
			constructor : function()
			{
				this.state = null;
				this.params = [];
				this.value = undefined;
				this.valueExpr = "";
			}
		})
	},

	constructor : function()
	{
		this.params = null;
		this.effects = [];
		this.restrictions = [];
		this.preconditions = [];
	},

	init : function()
	{
		for (var i = this.effects.length - 1; i >= 0; i--) 
		{
			//var tokens = this.effects[i].split(/\s*|\s*/);

			var stateInfo = this.parseState(this.effects[i]);
			this.effects[i] = stateInfo;
			this.addToState(stateInfo);
		};

		for (var i = this.preconditions.length - 1; i >= 0; i--) 
		{
			var stateInfo = this.parseState(this.preconditions[i]);
			this.preconditions[i] = stateInfo;
		};
	},

	parseState : function(stateExpr)
	{
		var stateInfo = new WorldActionDefinition.StateInfo();

		var tokens = stateExpr.split(/\s*:\s*/);

		if (tokens.length < 1)
			throw ("not enough tokens in state expression: " + stateExpr);

		if (tokens.length >= 2)
		{
			var valueExpr = tokens[1].trim();
			stateInfo.value = StateValue.parse(valueExpr);
			if (stateInfo.value === undefined)
				stateInfo.valueExpr = valueExpr;			
		}
		else
		{
			stateInfo.value = new StateValue(true);
		}

		tokens = tokens[0].split(/\s*[()]\s*/);
		if (tokens.length < 1)
			throw ("not enough tokens in state description: " + stateExpr);

		var stateName = tokens[0];
		stateInfo.state = Definition.get(stateName);

		if (tokens.length >= 2)
		{
			stateInfo.params = tokens[1].split(/\s*,\s*/);
		}

		return stateInfo;
	},

	addToState : function(effectInfo)
	{
		var actionInfo = new WorldStateDefinition.ActionInfo();
		actionInfo.action = this;
		
		paramExprs = {};
		directParams = [];

		//actionInfo.params.length = this.params.length;

		for (var i = 0; i < effectInfo.params.length; i++) 
		{
			if (this.params.has(effectInfo.params[i]))
			{
				var actionParam = effectInfo.params[i];
				var effectParam = effectInfo.state.params[i];
				paramExprs[actionParam] = effectParam;
				directParams.push(actionParam);
			}			
		};

		for (var paramName in this.restrictions)
		{
			if (!paramExprs.hasOwnProperty(paramName))
			{
				var expr = this.restrictions[paramName];

				for (var i = 0; i < directParams.length; i++) 
				{
					var re = new RegExp("\\b" + directParams[i] + "\\b");
					expr = expr.replace(re, paramExprs[directParams[i]]);					
				};

				paramExprs[paramName] = expr;
			}
		};

		for (var i = 0; i < this.params.length; i++) 
		{
			var name = this.params[i];
			if (!paramExprs.hasOwnProperty(name))
				throw ("Action parameters cannot be fully initialized! missing: " + name);

			actionInfo.params[i] = paramExprs[name];
		};

		effectInfo.state.addEffectiveAction(actionInfo);
	},

	instanceClass : null,

	instantiate : function(params, context)
	{
		return new this.instanceClass(this, params, context);
	}
});

WorldAction = Class(Planner.Action, 
{
	constructor : function(def, params, context)
	{
		WorldAction.$super.call(this);

		this.def = def;
		this.params = new ConstParams(def.params, params);
		this.populateStates(this.effects, def.effects, context);
		this.populateStates(this.preconditions, def.preconditions, context);
	},

	populateStates : function(states, statesInfo, context)
	{
		states.length = 0;

		this.params.attachTo(context);
		for (var i = 0; i < statesInfo.length; i++) 
		{
			var stateInfo = statesInfo[i];
			var params = Evaluator.evalList(stateInfo.params, context);
			var value = stateInfo.value !== undefined ? stateInfo.value : StateValue.parse(stateInfo.valueExpr, context);
			states.push(stateInfo.state.instantiate(params, value));
		};
		this.params.detachFrom(context);
	},

	equals : function(other)
	{
		return this.def === other.def && this.params.equals(other.params);
	}
});