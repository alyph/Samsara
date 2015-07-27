var _ = {};

new (function(global)
{
	var predicates = {};
	var predicateKeywords = {};

	var resolvers = {};

	var functable = _;
	//var _ = functable;
	var funcNamespace = "_";

	var Predicate = Class(
	{
		constructor : function()
		{
			this.params = null;
			this.func = null;
		},

		eval : function(keyValues)
		{
			var values = this.params.toValues(keyValues);
			return this.evalVargs.apply(this, values);
		},

		evalVargs : function(vargs)
		{
			if (arguments.length < this.params.length)
				throw ("less arguments than required.");

			return this.func.apply(this, arguments);
		},

		// evalParams : function(params, args)
		// {
		// 	var l = params.length;
		// 	var values = [];
		// 	values.length = l;
		// 	for (var i = 0; i < l; i++) 
		// 	{
		// 		values[i] = params[i].eval(args);
		// 	};

		// 	return this.evalVargs.apply(this, values);
		// }
	});

	var GlobalPredicate = Class(BaseObject,
	{
		constructor : function()
		{
			GlobalPredicate.$super.call(this);

			this.params = null;
			this.expression = "";
			this.funcName = "";
			this.isCompiled = false;
		},

		compile : function()
		{
			if (this.expression === null)
				throw ("no expression.");

			var result = parseExpression(this.expression);
			var func = result[1];
			this.doneCompile(func);
		},

		doneCompile : function(func)
		{
			var baseFuncName = this.funcName;
			var suffix = 1;
			while (functable.hasOwnProperty(this.funcName))
			{
				this.funcName = baseFuncName + (suffix++);
			};

			functable[this.funcName] = func;
			this.isCompiled = true;
		}
	});

	var Resolver = Class(
	{
		constructor : function()
		{
		},

		eval : function(vargs)
		{
			throw ("not implemented!");
		}
	});

	global.$predicate = createPredicate;
	function createPredicate(statement, expression)
	{
		var isGlobal = arguments.length >= 2;
		if (isGlobal)
		{
			var key = "";
			var params = [];
			var keywords = [];
			var tokens = statement.trim().split(/\s+/);
			var l = tokens.length;
			for (var i = 0; i < l; i++) 
			{
				var token = tokens[i];
				if (token.length === 0)
					continue;

				if (token[0] === '$')
				{
					params.push(token.substr(1));
					key += "?";
				}
				else
				{
					keywords.push(token);
					predicateKeywords[token] = true;
					key += token;
				}
			};

			if (predicates.hasOwnProperty(key))
			{
				throw ("Predicate key conflicts: " + key + ", Statement:" + statement + ", Other:" + predicates[key].expression);
			}

			var funcName = keywords.join('_');
			var predicate = new GlobalPredicate();
			predicate.params = params;
			predicate.expression = expression;
			predicate.funcName = funcName;
			predicates[key] = predicate;

			if (typeof expression === 'function')
			{
				predicate.doneCompile(expression);
			}
			else
			{
				predicate.expression = expression;
			}

			return predicate;
		}
		else // local predicate, only need a parameter list, since no other predicates will actually reference it.
		{
			var predicate = new Predicate();
			var result = parseExpression(statement);
			predicate.params = $p.apply(this, result[0]);
			predicate.func = result[1];
			return predicate;
		}
	};

	// function isCompositeKeyword(symbol)
	// {
	// 	throw ("not implemented!");
	// };

	function findCompiledPredicate(key)
	{
		var predicate = predicates[key] || null;
		if (predicate !== null && !predicate.isCompiled)
			predicate.compile();

		return predicate;
	};

	function findResolver(key)
	{
		return resolvers[key] || null;
	}

	// function isResolverPrefix(token)
	// {
	// 	throw ("not implemented!");
	// };

	// var PredicateExpression = Class(
	// {
	// 	constructor : function(predicate, params)
	// 	{
	// 		this.predicate = predicate;
	// 		this.params = params;
	// 	},

	// 	eval : function(args)
	// 	{			
	// 		return this.predicate.evalParams(this.params, args);
	// 	}
	// });

	// var ParameterExpression = Class(
	// {
	// 	constructor : function(constant, variables, func)
	// 	{
	// 		this.constant = constant || null;
	// 		this.variables = variables || null;
	// 		this.func = func || null;
	// 	},

	// 	eval : function(args)
	// 	{
	// 		if (this.variables === null)
	// 			return this.constant;

	// 		var l = this.variables.length;
	// 		var values = [];
	// 		values.length = l;
	// 		for (var i = 0; i < l; i++) 
	// 		{
	// 			values[i] = this.variables[i].eval(args);
	// 		};

	// 		return this.func !== null ? this.func.call(this, values) : values[0];
	// 	}
	// });

	// var VariableExpression = Class(
	// {
	// 	constructor : function(idIndex, resolvers)
	// 	{
	// 		this.idIndex = idIndex;
	// 		this.resolvers = resolvers;
	// 	},

	// 	eval : function(args)
	// 	{
	// 		if (this.argIdx >= args.length)
	// 			throw ("not enough input arguments.");

	// 		var output = args[this.idIndex];

	// 		if (this.resolvers)
	// 		{
	// 			var l = this.resolvers.length;
	// 			for (var i = 0; i < l; i++) 
	// 			{
	// 				output = this.resolvers[i].eval(output, args);
	// 			};
	// 		}

	// 		return output;
	// 	}
	// });

	// var ResolverExpression = Class(
	// {
	// 	constructor : function(resolver, params)
	// 	{
	// 		this.resolver = resolver;
	// 		this.params = params;
	// 	},

	// 	eval : function(input, args)
	// 	{
	// 		var resolverArgs = [];
	// 		resolverArgs.push(input);

	// 		if (this.params)
	// 		{
	// 			var l = this.params.length;
	// 			resolverArgs.length = l + 1;
	// 			for (var i = 0; i < l; i++) 
	// 			{
	// 				resolverArgs[i+1] = this.params[i].eval(args);
	// 			};
	// 		}

	// 		return this.resolver.eval.apply(this.resolver, resolverArgs);
	// 	}
	// });

	var TokenTypes =
	{
		keyword				: 0,
		composite 			: 1,
		identifier			: 2,
		openParenthesis		: 3,
		closeParenthesis	: 4,
		Comma 				: 5,
		end					: 6,
		quote 				: 7,
		string 				: 8,
		symbol 				: 9,
		resolver 			: 10,
		dot					: 11
	}

	var Token = Class(
	{
		constructor : function()
		{
			this.str = "";
			this.type = -1;
			this.index = 0;
		}
	});

	var TokenReader = Class(
	{
		constructor : function(tokens)
		{
			this.tokens = tokens;
			this.cursor = 0;
			this.identifiers = [];
		},

		peek : function()
		{
			return this.tokens[this.cursor];
		},

		next : function()
		{
			if (this.cursor === this.tokens.length - 1)
				throw ("cannot move to next. End of the token stream.");

			return this.tokens[this.cursor++];
		},

		expect : function(type)
		{
			var nextType = this.tokens[this.cursor].type;
			if (nextType !== type)
				throw ("Expecting " + type + " but encountering " + nextType);

			return this.next();
		},

		test : function(type)
		{
			if (this.tokens[this.cursor].type === type)
			{
				return this.next();
			}
		},

		analyzeSymbol : function(token)
		{
			if (token.type === TokenTypes.symbol)
			{
				var str = token.str;
				if (predicateKeywords.hasOwnProperty(str))
				{
					token.type = TokenTypes.keyword;
				}
				else if (findResolver(str) !== null)
				{
					token.type = TokenTypes.resolver;
				}
				else
				{
					token.type = TokenTypes.identifier;
					this.addIdentifier(str);
				}
			}
		},

		addIdentifier : function(identifier)
		{
			if (this.identifiers.indexOf(identifier) < 0)
			{
				this.identifiers.push(identifier);
			}
		}
	});

	// function CompiledFunction(args, body) 
	// {
	// 	return Function.apply(this, args.concat(body));
	// }
	// CompiledFunction.prototype = Function.prototype;

	function parseExpression(expression, params)
	{		
		var tokens = tokenize(expression, params);
		var reader = new TokenReader(tokens);
		var funcBody = "return (" + parsePredicate(reader) + ");";
		var args = reader.identifiers;

		if (params)
		{
			var l = args.length;
			for (var i = 0; i < l; i++) 
			{
				if (!params.has(args[i]))
					throw ("The identifier " + args[i] + " used in the expression " + expression + " are not found in the parameter list defined by the statement.");
			};

			args = [].concat(params);
		}

		return [args, Function.apply(null, args.concat(funcBody))];
	};

	function tokenize(expression, params)
	{
		var reg = /[a-zA-Z]\w*|-?\d+(?:\.\d+)?|[><!=]=|['+\-*\/(),.<>]/g;
		var tokens = [];

		var result;
		while ((result = reg.exec(expression)) !== null) 
		{
			var match = result[0];
			var token = new Token();
			token.str = match;
			tokens.push(token);

			// String
			if (match === "'")
			{
				token.type = TokenTypes.string;
				var closeQuote = expression.indexOf("'", reg.lastIndex);
				if (closeQuote < 0)
				{
					token.str = expression.substr(reg.lastIndex - 1) + "'";
					reg.lastIndex = expression.length;
				}
				else
				{
					token.str = expression.substring(reg.lastIndex - 1, closeQuote + 1);
					reg.lastIndex = closeQuote + 1;
				}
			}
			// Open Parenthesis
			else if (match === "(")
			{
				token.type = TokenTypes.openParenthesis;
			}
			// Close Parenthesis
			else if (match === ")")
			{
				token.type = TokenTypes.closeParenthesis;
			}
			// Comma
			else if (match === ",")
			{
				token.type = TokenTypes.comma;
			}
			// Dot
			else if (match === ".")
			{
				token.type = TokenTypes.dot;
			}
			// Symbol
			else
			{
				token.type = TokenTypes.symbol;
			}
		}

		var endToken = new Token();
		endToken.type = TokenTypes.end;
		tokens.push(endToken);

		return tokens;
	};

	function parsePredicate(reader)
	{
		var key = "";
		var params = [];
		
		var next = reader.peek();
		while (next.type !== TokenTypes.end &&
			next.type !== TokenTypes.comma &&
			next.type !== TokenTypes.closeParenthesis)
		{
			reader.analyzeSymbol(next);
			if (next.type === TokenTypes.keyword)
			{
				key += next.str;
				reader.next();
			}
			else
			{
				key += "?";
				params.push(parseParameter(reader));
			}

			next = reader.peek();
		}

		var predicate = findCompiledPredicate(key);
		if (predicate === null)
			throw ("cannot find predicate: " + key);

		return funcNamespace + "." + predicate.funcName + "(" + params.join(", ") + ")";
	};
/*
	function parseCompositePredicate(reader)
	{
		var code = reader.expect(TokenTypes.composite).str;
		var subPredicates = [];

		reader.expect(TokenTypes.openParenthesis);

		do
		{
			subPredicates.push(parsePredicate(reader));

		} while(reader.test(TokenTypes.comma));

		reader.expect(TokenTypes.closeParenthesis);

		return buildPredicateExpression(code, subPredicates);
	};*/

	// function buildPredicateExpression(code, params)
	// {
	// 	var predicate = findCompiledPredicate(code);
	// 	if (!predicate)
	// 		throw ("No predicate matching code: " + code);

	// 	return new PredicateExpression(predicate, params);
	// };

	function parseParameter(reader)
	{
		var parenthesisCount = 0;
		var script = "";

		while (true)
		{
			var next = reader.peek();

			if (next.type === TokenTypes.end || 
				next.type === TokenTypes.comma)				
			{
				break;
			}
			else if (next.type === TokenTypes.openParenthesis)
			{
				parenthesisCount++;
			}
			else if (next.type === TokenTypes.closeParenthesis)
			{
				if (--parenthesisCount < 0)
					break;
			}
			else
			{
				reader.analyzeSymbol(next);
				if (next.type === TokenTypes.keyword)
				{
					break;
				}
				else if (next.type === TokenTypes.identifier)
				{
					script += parseReference(reader);
					continue;
				}
				else if (next.type === TokenTypes.resolver)
				{
					script += parseResolver(reader);
					continue;
				}
			}

			script += reader.next().str;
		}

		return script;
	};

	// function buildParameterExpression(script, variables)
	// {
	// 	if (variables.length === 0)
	// 		return new ParameterExpression(eval(script), null, null);

	// 	if (script === "$vars[0]")
	// 		return new ParameterExpression(null, variables, null);

	// 	var func = new Function("vars", "return " + script + ";");
	// 	return new ParameterExpression(null, variables, func);
	// };

	function parseReference(reader)
	{
		var identifier = reader.expect(TokenTypes.identifier);
		var script = identifier.str;

		while (reader.test(TokenTypes.dot))
		{
			script = parseResolver(reader, script);
		}

		return script;
	};

	// function buildVariableExpression(identifier, resolvers)
	// {
	// 	return new VariableExpression(identifier.index, resolvers);
	// };

	function parseResolver(reader, self)
	{
		var func = "";
		var params = [];

		var resolverToken = reader.next();

		var resolver = findResolver(resolverToken.str);
		if (resolver !== null)
			func = resolver.funcName;

		if (arguments.length >= 2)
		{
			params.push(self);

			if (func === "")
			{
				func = "prop";
				params.push(resolverToken.str);
			}
		}

		if (func === "")
			throw ("Cannot find proper resolver function: " + resolverToken.str);


		if (reader.test(TokenTypes.openParenthesis))
		{			
			do
			{
				params.push(parsePredicate(reader));

			} while(reader.test(TokenTypes.comma));

			reader.expect(TokenTypes.closeParenthesis);
		}

		return funcNamespace + "." + func + "(" + params.join(", ") + ")";
	};

	// function buildResolverExpression(prefix, keyword, params)
	// {
	// 	var resolver = findResolver(prefix.str + keyword.str);
	// 	if (!resolver)
	// 		throw ("No resolver matching code: " + code);

	// 	return new ResolverExpression(resolver, params);
	// };
	
})(this);