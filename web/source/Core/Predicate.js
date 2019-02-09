var _ = {};

new (function(global)
{
	'use strict';

	var predicates = {};
	var predicateKeywords = {};

	var resolvers = {};

	var functable = _;
	var funcNamespace = "_";

	global.$predicate = createPredicate;
	global.$resolver = createResolver;

	class Predicate
	{
		constructor()
		{
			this.params = null;
			this.func = null;
		}

		eval(keyValues)
		{
			var values = this.params.toValues(keyValues);
			return this.evalVargs.apply(this, values);
		}

		evalVargs(vargs)
		{
			if (arguments.length < this.params.length)
				throw ("less arguments than required.");

			return this.func.apply(this, arguments);
		}
	}

	class GlobalPredicate
	{
		constructor()
		{
			this.params = null;
			this.expression = "";
			this.funcName = "";
			this.isCompiled = false;
		}

		compile()
		{
			if (this.expression === null)
				throw ("no expression.");

			var result = compileExpression(this.expression);
			var func = result[1];
			this.doneCompile(func);
		}

		doneCompile(func)
		{
			var baseFuncName = this.funcName;
			var suffix = 1;
			while (functable.hasOwnProperty(this.funcName))
			{
				this.funcName = baseFuncName + (suffix++);
			}

			functable[this.funcName] = func;
			this.isCompiled = true;
		}
	}

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
			}

			if (predicates.hasOwnProperty(key))
			{
				throw ("Predicate key conflicts: " + key + ", Statement:" + statement + ", Other:" + predicates[key].expression);
			}

			var funcName = keywords.join('_');
			let predicate = new GlobalPredicate();
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
			let predicate = new Predicate();
			var result = compileExpression(statement);
			predicate.params = $p.apply(null, result[0]);
			predicate.func = result[1];
			return predicate;
		}
	}

	function findCompiledPredicate(key)
	{
		var predicate = predicates[key] || null;
		if (predicate !== null && !predicate.isCompiled)
			predicate.compile();

		return predicate;
	}

	class Resolver
	{
		constructor()
		{
			this.funcName = "";
		}

		parse(params)
		{
			// TODO: verify the number of parameters
			return funcNamespace + "." + this.funcName + "(" + params.join(", ") + ")";
		}
	}

	// TODO: save prop into a var
	createResolver("prop", function(obj, key)
	{
		return obj ? obj[key] : undefined;
	}); 

	createResolver("all", null, function(params)
	{
		return "(" + params.join(") && (") + ")";
	});

	createResolver("any", null, function(params)
	{
		return "(" + params.join(") || (") + ")";
	});

	function createResolver(name, func, parser)
	{
		if (resolvers.hasOwnProperty(name))
			throw ("Resolver with name " + name + " is already taken.");

		var resolver = new Resolver();

		if (func)
		{
			resolver.funcName = assignToFuncTable(name, func);	
		}
		
		if (parser)
		{
			resolver.parse = parser;
		}

		resolvers[name] = resolver;
		return resolver;
	}

	function findResolver(key)
	{
		return resolvers[key] || null;
	}

	function assignToFuncTable(name, func)
	{
		var assignedName = name;
		var suffix = 1;
		while (functable.hasOwnProperty(assignedName))
		{
			assignedName = name + (suffix++);
		}

		functable[assignedName] = func;
		return assignedName;
	}

	var TokenTypes =
	{
		symbol 				: 0,
		keyword				: 1,
		resolver 			: 2,
		identifier			: 3,
		openParenthesis		: 4,
		closeParenthesis	: 5,
		dot					: 6,
		comma 				: 7,
		string 				: 8,
		end					: 9
	};

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

	function compileExpression(expression, params)
	{		
		var tokens = tokenize(expression, params);
		var reader = new TokenReader(tokens);
		var funcBody = "return (" + parseExpression(reader, true) + ");";
		var args = reader.identifiers;

		if (params)
		{
			var l = args.length;
			for (var i = 0; i < l; i++) 
			{
				if (!params.has(args[i]))
					throw ("The identifier " + args[i] + " used in the expression " + expression + " are not found in the parameter list defined by the statement.");
			}

			args = [].concat(params);
		}

		return [args, Function.apply(null, args.concat(funcBody))];
	}

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
	}

	function parsePredicate(reader, firstParam)
	{
		var key = "";
		var params = [];

		if (firstParam)
		{
			key = "?";
			params.push(firstParam);
		}
		
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
				params.push(parseExpression(reader, false));
			}

			next = reader.peek();
		}

		var predicate = findCompiledPredicate(key);
		if (predicate === null)
			throw ("cannot find predicate: " + key);

		return funcNamespace + "." + predicate.funcName + "(" + params.join(", ") + ")";
	}

	function parseExpression(reader, canBePredicate)
	{
		var script = "";

		while (true)
		{
			var next = reader.peek();

			if (next.type === TokenTypes.end || 
				next.type === TokenTypes.comma ||
				next.type === TokenTypes.closeParenthesis)				
			{
				break;
			}
			else if (next.type === TokenTypes.openParenthesis)
			{
				script += reader.next().str + parseExpression(reader, true);
				while (reader.test(TokenTypes.comma))
				{
					script += ", " + parseExpression(reader, true);
				}
				script += reader.expect(TokenTypes.closeParenthesis).str;
			}
			else
			{
				reader.analyzeSymbol(next);
				if (next.type === TokenTypes.keyword)
				{
					if (canBePredicate)
						script = parsePredicate(reader, script);
					else
						break;
				}
				else if (next.type === TokenTypes.identifier)
				{
					script += parseReference(reader);
				}
				else if (next.type === TokenTypes.resolver)
				{
					script += parseResolver(reader);
				}
				else
				{
					script += reader.next().str;
				}
			}
		}

		return script;
	}


	// function parseParameter(reader)
	// {
	// 	var parenthesisCount = 0;
	// 	var script = "";

	// 	while (true)
	// 	{
	// 		var next = reader.peek();

	// 		if (next.type === TokenTypes.end || 
	// 			next.type === TokenTypes.comma)				
	// 		{
	// 			break;
	// 		}
	// 		else if (next.type === TokenTypes.openParenthesis)
	// 		{
	// 			parenthesisCount++;
	// 		}
	// 		else if (next.type === TokenTypes.closeParenthesis)
	// 		{
	// 			if (--parenthesisCount < 0)
	// 				break;
	// 		}
	// 		else
	// 		{
	// 			reader.analyzeSymbol(next);
	// 			if (next.type === TokenTypes.keyword)
	// 			{
	// 				break;
	// 			}
	// 			else if (next.type === TokenTypes.identifier)
	// 			{
	// 				script += parseReference(reader);
	// 				continue;
	// 			}
	// 			else if (next.type === TokenTypes.resolver)
	// 			{
	// 				script += parseResolver(reader);
	// 				continue;
	// 			}
	// 		}

	// 		script += reader.next().str;
	// 	}

	// 	return script;
	// }

	function parseReference(reader)
	{
		var identifier = reader.expect(TokenTypes.identifier);
		var script = identifier.str;

		while (reader.test(TokenTypes.dot))
		{
			script = parseResolver(reader, script);
		}

		return script;
	}

	function parseResolver(reader, self)
	{
		//var func = "";
		var params = [];
		var resolverToken = reader.next();
		var resolver = findResolver(resolverToken.str);
		// if (resolver !== null)
		// 	func = resolver.funcName;

		if (arguments.length >= 2)
		{
			params.push(self);

			if (resolver === null)
			{
				resolver = findResolver("prop");
				params.push(resolverToken.str);
			}
			// if (func === "")
			// {
			// 	func = "prop";
			// 	params.push(resolverToken.str);
			// }
		}

		if (resolver === null)
			throw ("Cannot find proper resolver: " + resolverToken.str);


		if (reader.test(TokenTypes.openParenthesis))
		{			
			do
			{
				params.push(parseExpression(reader, true));

			} while(reader.test(TokenTypes.comma));

			reader.expect(TokenTypes.closeParenthesis);
		}

		return resolver.parse(params);
	}
	
})(this);

