/* globals -GLP */
/* exported GLP */

// General Language Parser
var GLP =  new (function(global) 
{
	var _eof = newTokenType("eof");

	this.parse = function(input, lexer, parser)
	{
		let stream = new InputStream(input);
		stream.pushLexer(lexer);
		return parser(stream);
	};

	this.TokenTypes = function(tokenNames)
	{
		let types = {};
		for (let name of tokenNames)
		{
			types[name] = newTokenType(name);
		}
		return types;
	};

	class InputStream
	{
		constructor(input)
		{
			this.str = input;
			this.cursor = 0;
			this.lexers = [];
		}

		pushLexer(lexer)
		{
			this.lexers.push(lexer);
		}

		peek()
		{
			if (this.cursor >= this.str.length)
				return eof();

			if (this.lexers.length === 0)
				throw ("No lexer.");

			let lexer = this.lexers[this.lexers.length - 1];

			let { tokenType, tokenStr, index } = lexer.search(this.str, this.cursor);

			if (tokenStr)
			{
				return new Token(tokenType, tokenStr, index);
			}
			else
			{
				return eof();
			}
		}

		next()
		{
			let token = this.peek();
			if (token.isEof())
			{
				this.cursor = this.str.length;
			}
			else
			{
				if (token.index < this.cursor)
					throw ("Found token index ahead of the cursor.");

				this.cursor = token.index + token.str.length;
			}

			return token;
		}

		expect(type, str)
		{
			// TODO: just call skip() and error if returned null?
			let token = this.peek();
			if (token.is(type, str))
			{
				return this.next();
			}
			else
			{
				this.error(`Expected a token ${type.toString()}${str?` '${str}'`:""} and instead saw ${token}.`, token);
				return null;
			}
		}

		skip(type, str)
		{
			let token = this.peek();
			if (token.is(type, str))
			{
				return this.next();
			}
			else
			{
				return null;
			}
		}

		match(regex)
		{
			regex.lastIndex = this.cursor;
			let result = regex.exec(this.str);
			if (result && result.index === this.cursor)
			{
				let tokenStr = result[0];

				this.cursor = (result.index + tokenStr.length);

				// Each matched regex token will have its own unique token type, 
				// so will never match any existing token types.
				return new Token(newTokenType("regex"), tokenStr, result.index);
			}
			else
			{
				this.error(`Regex match failed: ${regex}.`);
				return null;
			}
		}

		error(msg, token)
		{
			let pos = token ? token.index : this.cursor;
			console.error(`[${pos}]: ${msg}`); // TODO: include file and line#
		}
	}

	class Token
	{
		constructor(type, str, index)
		{
			this.type = type;
			this.str = str;
			this.index = index;
		}

		is(type, str)
		{
			return this.type === type && 
				(str === undefined || this.str === str);
		}

		isEof()
		{
			return this.type === _eof;
		}

		toString()
		{
			return this.type.toString() + (this.str ? ` '${this.str}'` : "");
		}
	}

	function newTokenType(name)
	{
		return Symbol(name);
	}

	function eof()
	{
		return new Token(_eof, "", -1);
	}

	this.CommonRegexLexer = class CommonRegexLexer
	{
		constructor(regexToTokens)
		{
			this.tokenTypes = [];

			let groups = [];
			for (let [regex, tokenType = null] of regexToTokens)
			{
				if (tokenType)
				{
					groups.push(`(${regex})`); // capture
					this.tokenTypes.push(tokenType);
				}
				else
				{
					groups.push(`(?:${regex})`); // non-capture
				}
			}

			this.regex = new RegExp(groups.join('|'), 'g');
		}

		search(input, start)
		{
			this.regex.lastIndex = start;

			let result;
			while ((result = this.regex.exec(input)))
			{
				let tokenType = null;
				for (let i = 0; i < this.tokenTypes.length; i++) 
				{
					if (result[i+1])
					{
						tokenType = this.tokenTypes[i];
						break;
					}
				}

				if (tokenType)
				{
					return { tokenType: tokenType, tokenStr: result[0], index: result.index };
				}
				// May match none, in that case, it hits the skipped tokens (e.g. comments), then continue.

				// Sanity check.
				if (this.regex.lastIndex <= result.index)
					throw (`Matched empty string? ${result[0]}`);
			}

			// No valid match
			return { tokenType: null, tokenStr: "", index: -1 };
		}
	};

})(this);
