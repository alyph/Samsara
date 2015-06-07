function(global)
{
	var predicates = {};
	var resolvers = {};

	var Predicate = Class(
	{
		constructor : function()
		{
			this.params = null;
			this.expression = null;
		},

		eval : function(vargs)
		{
			if (arguments.length < this.params.length)
				throw ("less arguments than required.");

			return this.expression.eval(arguments);
		},

		evalParams : function(params, args)
		{
			var l = params.length;
			var values = [];
			values.length = l;
			for (var i = 0; i < l; i++) 
			{
				values[i] = params[i].eval(args);
			};

			return this.eval.apply(this, values);
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

	function createPredicate(name, statement, expression)
	{
		var isGlobal = arguments.length >= 3;
		var params = [];
		if (isGlobal)
		{
			var tokens = statement.trim().split(/\s+/);
			var l = tokens.length;
			for (var i = 0; i < l; i++) 
			{
				var token = tokens[i];
				if (token.length > 0 && token[0] === '$')
					params.push(token.substr(1));
			};

			// TODO: save into global table, with the statement format.
			// convert statement to code
			throw ("not implemented!");
		}
		else // local predicate, only need a parameter list, since no other predicates will actually reference it.
		{
			expression = statement;
			params = statement.trim().split(/\s*,\s*/);
		}

		var props = {};
		props.$base = Predicate;
		props.params = $p.apply(global, params);
		props.expression = $v([params, expression], parseExpression);

		if (isGlobal) // For global predicate, load as a definition, so later Archive will process it.
		{
			$def(name, props);	
		}
		else // For local predicate, simply return the properties to whomever is referencing it.
		{
			return props;
		}
	};

	function findPredicate(code)
	{
		return predicates[code] || null;
	};

	function findResolver(code)
	{
		return resolvers[code] || null;
	}

	function isResolverPrefix(token)
	{
		throw ("not implemented!");
	};

	var PredicateExpression = Class(
	{
		constructor : function(predicate, params)
		{
			this.predicate = predicate;
			this.params = params;
		},

		eval : function(args)
		{			
			return this.predicate.evalParams(this.params, args);
		}
	});

	var ParameterExpression = Class(
	{
		constructor : function(constant, variables, func)
		{
			this.constant = constant || null;
			this.variables = variables || null;
			this.func = func || null;
		},

		eval : function(args)
		{
			if (this.variables === null)
				return this.constant;

			var l = this.variables.length;
			var values = [];
			values.length = l;
			for (var i = 0; i < l; i++) 
			{
				values[i] = this.variables[i].eval(args);
			};

			return this.func !== null ? this.func.call(this, values) : values[0];
		}
	});

	var VariableExpression = Class(
	{
		constructor : function(idIndex, resolvers)
		{
			this.idIndex = idIndex;
			this.resolvers = resolvers;
		},

		eval : function(args)
		{
			if (this.argIdx >= args.length)
				throw ("not enough input arguments.");

			var output = args[this.idIndex];

			if (this.resolvers)
			{
				var l = this.resolvers.length;
				for (var i = 0; i < l; i++) 
				{
					output = this.resolvers[i].eval(output, args);
				};
			}

			return output;
		}
	});

	var ResolverExpression = Class(
	{
		constructor : function(resolver, params)
		{
			this.resolver = resolver;
			this.params = params;
		},

		eval : function(input, args)
		{
			var resolverArgs = [];
			resolverArgs.push(input);

			if (this.params)
			{
				var l = this.params.length;
				resolverArgs.length = l + 1;
				for (var i = 0; i < l; i++) 
				{
					resolverArgs[i+1] = this.params[i].eval(args);
				};
			}

			return this.resolver.eval.apply(this.resolver, resolverArgs);
		}
	});

	var TokenTypes =
	{
		keyword: 0,
		composite: 1,
		identifier: 2,
		openParenthesis: 3,
		closeParenthesis: 4,
		comma: 5,
		end: 6,
		quote: 7
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
		},

		peek : function()
		{
			throw ("not implemented!");
		},

		next : function()
		{
			throw ("not implemented!");
		},

		expect : function(type)
		{
			throw ("not implemented!");
		},

		test : function(type)
		{
			throw ("not implemented!");
		}
	});

	function parseExpression(binder, record)
	{
		var source = binder.source;
		var params = source[0];
		var expression = source[1];
		var tokens = tokenize(expression, params);
		var reader = new TokenReader(tokens);
		var predicateExpr = parsePredicate(reader);

		// TODO: good to go?
		return predicateExpr;
	};

	function tokenize(expression, params)
	{

	};

	function parsePredicate(reader)
	{
		var next = reader.peek();
		if (next.type === TokenTypes.composite)
		{
			return parseCompositePredicate(reader);
		}
		else
		{
			var code = "";
			var params = [];
			while (next.type !== TokenTypes.end &&
				next.type !== TokenTypes.comma &&
				next.type !== TokenTypes.closeParenthesis)
			{
				if (next.type === TokenTypes.keyword)
				{
					code += next.str;
					reader.next();
				}
				else
				{
					code += "?";
					params.push(parseParameter(reader));
				}

				next = reader.peek();
			}

			return buildPredicateExpression(code, params);
		}
	};

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
	};

	function buildPredicateExpression(code, params)
	{
		var predicate = findPredicate(code);
		if (!predicate)
			throw ("No predicate matching code: " + code);

		return new PredicateExpression(predicate, params);
	};

	function parseParameter(reader)
	{
		var parenthesisCount = 0;
		var script = "";
		var openQuote = false;
		var variables = [];

		while (true)
		{
			var next = reader.peek();

			if (next.type === TokenTypes.quote)
			{
				script += reader.next().str;
				openQuote = !openQuote;
			}
			else if (openQuote)
			{
				script += reader.next().str;
			}
			else if (next.type === TokenTypes.identifier)
			{
				script += "$vars[" + variables.length + "]";
				variables.push(parseVariable(reader));
			}
			else
			{
				if (next.type === TokenTypes.end || next.type === TokenTypes.keyword)				
				{
					break;
				}
				else if (next.type === TokenTypes.comma)
				{
					if (parenthesisCount <= 0)
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

				script += reader.next().str;
			}
		}

		return buildParameterExpression(script, variables);
	};

	function buildParameterExpression(script, variables)
	{
		if (variables.length === 0)
			return new ParameterExpression(eval(script), null, null);

		if (script === "$vars[0]")
			return new ParameterExpression(null, variables, null);

		var func = new Function("vars", "return " + script + ";");
		return new ParameterExpression(null, variables, func);
	};

	function parseVariable(reader)
	{
		var identifier = reader.expect(TokenTypes.identifier);
		var resolvers = [];

		while (isResolverPrefix(reader.peek()))
		{
			resolvers.push(parseResolver(reader));
		}

		return buildVariableExpression(identifier, resolvers);
	};

	function buildVariableExpression(identifier, resolvers)
	{
		return new VariableExpression(identifier.index, resolvers);
	};

	function parseResolver(reader)
	{
		var prefix = reader.next();
		var keyword = reader.expect(TokenTypes.keyword);
		var params = null;

		if (reader.test(TokenTypes.openParenthesis))
		{
			params = [];

			do
			{
				params.push(parseParameter(reader));

			} while(reader.test(TokenTypes.comma));

			reader.expect(TokenTypes.closeParenthesis);
		}

		return buildResolverExpression(prefix, keyword, params);
	};

	function buildResolverExpression(prefix, keyword, params)
	{
		var resolver = findResolver(prefix.str + keyword.str);
		if (!resolver)
			throw ("No resolver matching code: " + code);

		return new ResolverExpression(resolver, params);
	};
	
}(this);