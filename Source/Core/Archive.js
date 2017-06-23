'use strict';

/* globals GLP */
/* exported Archive */

var Archive = new (function(global) 
{
	var _loadedPackages = new Set();
	var _records = new Map();
	var _objectToRecords = new Map();
	var _instances = [];
	var _rules = new Map();
	var _customValueTypes = [];
	var _packageLexer = null;
	var _baseUrl = "";

	var Environment =
	{
		definitions: "d",
		instances: "i",
	};

	var PackageExtension = 
	{
		[Environment.definitions]: 'def',
		[Environment.instances]: 'ins',
	};

	var ArchiveSymbol = 
	{
		loaded: Symbol("recordLoaded"),
	};
	this.Symbol = ArchiveSymbol;

	this.instances = _instances;

	this.init = function(config)
	{
		_baseUrl = config.baseUrl || "";
		if (_baseUrl.endsWith('/'))
			_baseUrl = _baseUrl.slice(0, -1);
	};

	this.Rule = function(name, parser)
	{
		if (_rules.has(name))
			throw (`Rule named '${name}' already exists.`);

		_rules.set(name, new Rule(parser));
	};

	this.CustomValueType = function(parser, compiler)
	{
		// TODO: may want to check duplicates
		_customValueTypes.push(new CustomValueType(parser, compiler));
	};

	this.loadInstances = function(path)
	{
		let pkgPath = normalizePath(path, Environment.instances);
		return loadPackage(pkgPath);
	};

	this.create = function(base, name = "")
	{
		let baseName = "";

		if (typeof base === 'function')
		{
			baseName = base.name;
		}
		else if (typeof base === 'object')
		{
			let record = _objectToRecords.get(base);
			if (record)
			{
				baseName = record.fullName;
			}
		}
		else if (typeof base === 'string')
		{
			baseName = base;
		}
		
		if (!baseName)
		{
			console.error(`Invalid base "${base}", failed to create new object.`);
			return null;
		}

		let instName = name;
		if (!instName)
		{
			let idx = baseName.lastIndexOf(Punctuation.nameDelim);
			if (idx >= 0)
			{
				instName = baseName.substr(idx+1);
			}
			else if ((idx = baseName.lastIndexOf(Punctuation.envDelim)) >= 0)
			{
				instName = baseName.substr(idx+1);
			}
			else
			{
				instName = baseName;
			}
		}

		if (!instName)
		{
			console.error(`Invalid instance name "${instName}", failed to create new object.`);
			return null;
		}

		instName = findUniqueRecordName(Environment.instances + Punctuation.envDelim + instName);

		let workingRec = new WorkingRecord();
		workingRec.fullName = instName;
		workingRec.scope.environment = Environment.instances;
		workingRec.dependencies[baseName] = null;
		workingRec.blueprint = new Blueprint();
		workingRec.blueprint.baseName = baseName;

		processRecords([workingRec]);

		let rec = _records.get(instName);
		return rec ? rec.object : null;
	};

	this.destroy = function(inst)
	{
		let record = _objectToRecords.get(inst);
		if (!record)
		{
			console.error(`Trying to destroy an instance (${inst}) that does not exist.`);
			return;
		}

		if (record.environment !== Environment.instances)
		{
			console.error(`Trying to destroy an object (${record.fullName}) that is not an instance.`);
			return;
		}

		removeRecord(record);
	};

	this.getInst = function(name)
	{
		let fullName = `${Environment.instances}${Punctuation.envDelim}${name}`;
		return getObject(fullName);
	};

	this.getDef = function(name)
	{
		let fullName = `${Environment.definitions}${Punctuation.envDelim}${name}`;
		return getObject(fullName);
	};



	function getObject(fullName)
	{
		let rec = _records.get(fullName);
		if (!rec)
			throw (`Cannot find record: ${fullName}`);

		return rec.object;
	}

	function addRecord(record)
	{
		if (_records.has(record.fullName))
			throw (`Adding duplicated record: ${record.fullName}`);

		_records.set(record.fullName, record);
		_objectToRecords.set(record.object, record);

		if (record.environment === Environment.instances)
			_instances.push(record.object);
	}

	function removeRecord(record)
	{
		_records.delete(record.fullName);
		_objectToRecords.delete(record.object);

		if (record.environment === Environment.instances)
		{
			let idx = _instances.indexOf(record.object);
			if (idx >= 0)
			{
				_instances.splice(idx, 1);
			}
		}
	}

	function findUniqueRecordName(name)
	{
		let max = 0x1000000000000; // 2^48
		let numBytes = 6;
		let bytes = new Uint8Array(numBytes);
		let lowerMask = 0x00FFFFFF;
		let mask = 0xFF;
		let uniqueName = "";

		do
		{
			let rand = Math.floor(Math.random() * max);
			let lower = rand & lowerMask;
			let higher = Math.floor((rand - lower) / (lowerMask + 1));

			// little endian, higher to lower bits.
			for (let i = numBytes - 1; i >= numBytes/2; i--) 
			{
				bytes[i] = (lower & mask);
				lower >>= 8; // Shift one byte
			}

			for (let i = numBytes/2 - 1; i >= 0; i--) 
			{
				bytes[i] = (higher & mask);
				higher >>= 8; // Shift one byte
			}

			let byteStr = String.fromCharCode.apply(null, bytes);
			let encoding = btoa(byteStr);
			uniqueName = `${name}_${encoding}`;

		} while(_records.has(uniqueName));

		return uniqueName;
	}




// ██╗      ██████╗  █████╗ ██████╗ ██╗███╗   ██╗ ██████╗ 
// ██║     ██╔═══██╗██╔══██╗██╔══██╗██║████╗  ██║██╔════╝ 
// ██║     ██║   ██║███████║██║  ██║██║██╔██╗ ██║██║  ███╗
// ██║     ██║   ██║██╔══██║██║  ██║██║██║╚██╗██║██║   ██║
// ███████╗╚██████╔╝██║  ██║██████╔╝██║██║ ╚████║╚██████╔╝
// ╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ 



	class PackageDownloadWorkSet
	{
		constructor()
		{
			this.pendingPackages = new Set();
			this.finishedPackages = {};
		}
	}

	function loadPackage(path)
	{
		let downloadAll = new Promise(function(resolve, reject)
		{
			let downloadSet = new PackageDownloadWorkSet();
			downloadPackage(path, downloadSet, resolve, reject);
		});

		return downloadAll.then(function(packages)
		{
			return processPackages(packages);
		});
	}

	function downloadPackage(path, workSet, resolve, reject)
	{
		//path = normalizePath(path);
		workSet.pendingPackages.add(path);
		let url = convertPathToUrl(path);
		ajax(url).then(function(data)
		{
			// parse package
			let { records, imports } = compilePackage(data, path);

			// any more to import?
			for (let importPath of imports)
			{
				if (!_loadedPackages.has(importPath) &&
					!workSet.pendingPackages.has(importPath) &&
					!workSet.finishedPackages[importPath])
				{
					downloadPackage(importPath, workSet, resolve, reject);
				}
			}

			// all loaded (if so, resolve the workset)
			workSet.pendingPackages.delete(path);
			workSet.finishedPackages[path] = records;

			if (workSet.pendingPackages.size === 0)
			{
				resolve(workSet.finishedPackages);
			}
		},

		function(reason)
		{
			// TODO: can we cancel other pending packages?
			reject(new Error(`Failed to download package "${url}": ${reason}`));
		});
	}

	function normalizePath(path, env)
	{
		if (!isAbsolutePath(path))
			path = '/' + path;

		let extIdx = path.lastIndexOf('.');
		if (extIdx >= 0)
			path = path.slice(0, extIdx);

		path += `.${PackageExtension[env]}`;

		return path;
	}

	function convertPathToUrl(path)
	{
		return _baseUrl + path; //+ ".apk";
	}

	function isAbsolutePath(path)
	{
		return path.startsWith('/');
	}

	function convertRelativePathToAbsolute(relativePath, relativeToPackage)
	{
		if (isAbsolutePath(relativePath))
			throw (`The path is not relative: ${relativePath}`);

		let idx = relativeToPackage.lastIndexOf('/');
		if (idx < 0)
			throw (`The other package path is also relative path, cannot be used to convert a relative path: ${relativeToPackage}`);

		return relativeToPackage.slice(0, idx+1) + relativePath;
	}

	function getEnvFromPackagePath(path)
	{
		for (let env of Object.values(Environment))
		{
			if (path.endsWith(`.${PackageExtension[env]}`))
				return env;
		}

		throw (`The package does not have valid extension: ${path}`);
	}

	function ajax(url)
	{
		let promise = new Promise(function(resolve, reject)
		{
			// Instantiates the XMLHttpRequest
			let request = new XMLHttpRequest();			
			request.open("GET", url, true);
			request.onreadystatechange = function()
			{
				if (request.readyState === XMLHttpRequest.DONE)
				{
					if (request.status === 200)
					{
						resolve(request.response);
					}
					else
					{
						reject(new Error(`Failed to GET ${url}, error code: ${request.status}`));
					}
				}
			};

			request.send();
		});

		return promise;
	}





// ██████╗  █████╗ ██████╗ ███████╗██╗███╗   ██╗ ██████╗ 
// ██╔══██╗██╔══██╗██╔══██╗██╔════╝██║████╗  ██║██╔════╝ 
// ██████╔╝███████║██████╔╝███████╗██║██╔██╗ ██║██║  ███╗
// ██╔═══╝ ██╔══██║██╔══██╗╚════██║██║██║╚██╗██║██║   ██║
// ██║     ██║  ██║██║  ██║███████║██║██║ ╚████║╚██████╔╝
// ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ 

	var TokenTypes = GLP.TokenTypes(
	[	
		"key",
		"identifier",
		"string",
		"number",
		"punctuation",
		"operators",		
	]);

	var Punctuation =
	{
		objectOpen		: '{',
		objectClose		: '}',
		listOpen		: '[',
		listClose		: ']',
		parenOpen		: '(',
		parenClose		: ')',
		envDelim		: '>',
		nameDelim		: '.',
		itemDelim		: ',',
		rule			: '@',
		customValue		: '@',
		reference		: '#',
		object			: '.', // TODO: should rename this to type
		member			: '.',
		assignment		: ':',
	};

	var BinaryOperators = ['+', '-', '*', '/', '%', '**'];
	var UnaryOperators = ['-'];

	class SyntaxNodePackage 			{constructor() { this.entries = []; }}
	class SyntaxNodeProperty 			{constructor() { this.key = ""; this.value = null; }}
	class SyntaxNodeObject 				{constructor() { this.base = null; this.properties = []; }}
	class SyntaxNodeObjectReference 	{constructor() { this.name = ""; }}
	class SyntaxNodeList 				{constructor() { this.entries = []; }}
	class SyntaxNodeCustomValue			{constructor() { this.value = null; this.type = null; }}
	class SyntaxNodeString 				{constructor() { this.value = ""; }}
	class SyntaxNodeNull 				{constructor() {}}
	class SyntaxNodeExpression 			{constructor() { this.value = ""; }}
	class SyntaxNodeRule				{constructor() { this.executor = null; }}

	_packageLexer = (() =>
	{
		let identifierRegex = String.raw`[a-zA-Z_]\w*`;
		//let keyRegex = identifierRegex + String.raw`\s*` + Punctuation.assignment;

		let punctuationSymbols = [];
		for (let key in Punctuation) 
		{
			punctuationSymbols.push(Punctuation[key]); 
		}
		let punctuationRegex = generateRegexFromSymbols(punctuationSymbols);

		let operatorsSymbols = BinaryOperators.slice();
		for (let op of UnaryOperators) 
		{
			if (!operatorsSymbols.includes(op))
				operatorsSymbols.push(op);
		}
		let operatorsRegex = generateRegexFromSymbols(operatorsSymbols);

		let regexToTokens =
		[
			//[keyRegex,										TokenTypes.key],
			[identifierRegex,								TokenTypes.identifier],
			[String.raw`\/\/.*`					], // single line comment (skipped)
			[String.raw`\/\*(?:.|[\r\n])*?\*\/`	], // multi line comment (skipped)
			[String.raw`"(?:[^"\\\n\r]|\\.)*"`,				TokenTypes.string],
			[String.raw`\d+\.?\d*|\.\d+`,					TokenTypes.number],
			[punctuationRegex,								TokenTypes.punctuation],
			[operatorsRegex,								TokenTypes.operators],
		]; 

		return new GLP.CommonRegexLexer(regexToTokens);
	})();

	function generateRegexFromSymbols(symbols)
	{
		let groups = [];

		for (let symbol of symbols)
		{
			let group = "";
			for (let char of symbol)
			{
				group += `\\${char}`;
			}
			groups.push(group);
		}

		return groups.join('|');
	}

	function parsePackage(input)
	{
		let pkgNode = new SyntaxNodePackage();
		let token, entry;
		while (!(token = input.peek()).isEof())
		{
			if (token.type === TokenTypes.identifier)
			{
				entry = parseProperty(input);
				if (entry && entry.value.constructor === SyntaxNodeObject)
				{
					pkgNode.entries.push(entry);
				}
				else
				{
					input.error(`Encountered invalid or non-object property at package root level.`, token);
					// don't fail just yet, continue attempt to parse the rest.
				}
			}
			else if (token.is(TokenTypes.punctuation, Punctuation.rule))
			{
				input.next();
				if ((entry = parseRule(input)))
					pkgNode.entries.push(entry);
			}
			else
			{
				input.error(`Unexpected token ${token} in package, must start with key (missing :?) or rules, package parsing will now be terminated (any content parsed earlier will be returned).`, token);
				break;
			}
		}

		return pkgNode;
	}

	function parseProperty(input)
	{
		let key = input.expect(TokenTypes.identifier);
		if (!key)
			return null;

		if (!input.expect(TokenTypes.punctuation, Punctuation.assignment))
			return null;

		let value = parseValue(input); 
		if (!value) // error reported (likely more errors to come)
			return null;

		let propNode = new SyntaxNodeProperty();				
		propNode.key = key.str.trim();//.slice(0, -1).trim();
		propNode.value = value;

		return propNode;
	}

	function parseValue(input)
	{
		let token = input.peek();

		// object, reference or custom value
		if (token.is(TokenTypes.punctuation, Punctuation.object))
		{
			input.next();
			let ref = parseObjectReference(input);
			if (ref)
			{
				return parseObject(input, ref);
			}
		}
		else if (token.is(TokenTypes.punctuation, Punctuation.reference))
		{
			input.next();
			return parseObjectReference(input);
		}
		else if (token.is(TokenTypes.punctuation, Punctuation.customValue))
		{
			input.next();
			return parseCustomValue(input);
		}
		else if (token.is(TokenTypes.punctuation, Punctuation.objectOpen))
		{
			return parseObject(input, null);
		}
		// list
		else if (token.is(TokenTypes.punctuation, Punctuation.listOpen))
		{
			return parseList(input);
		}		
		// math expression
		else if (token.type === TokenTypes.number ||
			token.type === TokenTypes.operators ||
			token.is(TokenTypes.punctuation, Punctuation.parenOpen))
		{
			return parseExpression(input);
		}
		// string
		else if (token.type === TokenTypes.string)
		{
			input.next();
			let strNode = new SyntaxNodeString();

			// Trim quotes.
			strNode.value = token.str.slice(1, -1); // TODO: clean up escaped chars
			return strNode;
		}
		// potentially null or string? (allow single word string？)
		else if (token.is(TokenTypes.identifier, "null"))
		{
			return new SyntaxNodeNull();
		}
		else if (token.type === TokenTypes.identifier)
		{
			return parseExpression(input);
		}
		else
		{
			input.error(`Unexpected token ${token}, the property value cannot be parsed.`, token);
		}
		return null;
	}

	function parseObjectReference(input)
	{
		let fullName = parseRecordName(input);
		if (!fullName)
			return null;

		let node = new SyntaxNodeObjectReference();
		node.name = fullName;
		return node;
	}

	function parseRecordName(input)
	{
		let fullName = "";
		let token = input.expect(TokenTypes.identifier);
		if (!token)
			return null;
		fullName += token.str;

		token = input.peek();
		if (token.type === TokenTypes.punctuation && 
			token.str === Punctuation.envDelim)
		{
			fullName += token.str;
			token.next();

			if (!(token = input.expect(TokenTypes.identifier)))
				return null;
			fullName += token.str;
		}

		while ((token = input.peek()).is(TokenTypes.punctuation, Punctuation.nameDelim))
		{
			fullName += token.str;
			input.next();

			if (!(token = input.expect(TokenTypes.identifier)))
				return null;
			fullName += token.str;	
		}

		return fullName;
	}

	function parseObject(input, base)
	{
		let objNode = new SyntaxNodeObject();
		objNode.base = base; // this may be null (in that case base is ommited and may be auto applied during later process)
		if (base && !base.name)
		{
			input.error(`Object has base but base's name is empty.`);
			return null;
		}

		if (!input.expect(TokenTypes.punctuation, Punctuation.objectOpen))
			return null;

		while (!input.peek().is(TokenTypes.punctuation, Punctuation.objectClose))
		{
			let prop = parseProperty(input);
			if (!prop)
				return null;

			objNode.properties.push(prop);

			input.skip(TokenTypes.punctuation, Punctuation.itemDelim);
		}

		if (!input.expect(TokenTypes.punctuation, Punctuation.objectClose))
			return null;

		return objNode;
	}

	function parseList(input)
	{
		let listNode = new SyntaxNodeList();

		if (!input.expect(TokenTypes.punctuation, Punctuation.listOpen))
			return null;

		while (!input.peek().is(TokenTypes.punctuation, Punctuation.listClose))
		{
			let entry = parseValue(input);
			if (!entry)
				return null;

			listNode.entries.push(entry);

			input.skip(TokenTypes.punctuation, Punctuation.itemDelim);
		}

		if (!input.expect(TokenTypes.punctuation, Punctuation.listClose))
			return null;

		return listNode;
	}

	function parseExpression(input)
	{
		let value = parseExpressionValue(input);
		if (!value)
			return null;

		let node = new SyntaxNodeExpression();
		node.value = value;
		return node;
	}

	function parseExpressionValue(input)
	{
		let terms = [];
		let term = parseExpressionTerm(input);
		if (!term)
			return null;

		terms.push(term);

		let token;
		while ((token = input.peek()).is(TokenTypes.operators))
		{
			if (!BinaryOperators.includes(token.str))
				break;

			input.next();
			terms.push(token.str);

			let term = parseExpressionTerm(input);
			if (!term)
				return null;
			terms.push(term);
		}

		return terms.join(' ');
	}

	function parseExpressionTerm(input)	
	{
		let token = input.peek();
		if (token.type === TokenTypes.number ||
			token.type === TokenTypes.string) // TODO: not entirely correct, does not support expression starting with string
		{
			return input.next().str;
		}
		else if (token.type === TokenTypes.identifier)
		{
			let refStr = input.next().str;
			while (input.peek().is(TokenTypes.punctuation, Punctuation.member))
			{
				refStr += input.next().str;
				let memberToken = input.expect(TokenTypes.identifier);
				if (!memberToken)
					return null;
				refStr += memberToken.str;
			}

			// A function
			if (input.peek().is(TokenTypes.punctuation, Punctuation.parenOpen))
			{
				// TODO: this only works for a single parameter, must support comma in parseExpressionValue somehow.
				refStr += parseExpressionTerm(input); // (v)
			}
			return refStr;
		}
		else if (token.type === TokenTypes.operators &&
			UnaryOperators.includes(token.str))
		{
			let op = input.next().str;
			let term = parseExpressionTerm(input);
			return term ? `${op} ${term}` : null;
		}
		else if (token.is(TokenTypes.punctuation, Punctuation.parenOpen))
		{
			input.next();
			let expr = parseExpressionValue(input);
			if (!expr)
				return null;

			if (!input.expect(TokenTypes.punctuation, Punctuation.parenClose))
				return null;

			return Punctuation.parenOpen + expr + Punctuation.parenClose;
		}
		else
		{
			input.error(`Unexpected token ${token}, the expression cannot be parsed.`, token);
			return null;
		}
	}

	function parseCustomValue(input)
	{
		for (let customType of _customValueTypes)
		{
			let result = customType.parser(input);
			if (result !== undefined) // undefined means the parser determines this is not its type.
			{
				if (result !== null)
				{
					let customNode = new SyntaxNodeCustomValue();
					customNode.value = result;
					customNode.type = customType;
					return customNode;
				}
				else // Error during parsing, but already reported.
				{
					return null;
				}
			}
		}

		input.error(`Cannot find a proper parser to parse the custom value.`);
		return null;
	}

	class Rule
	{
		constructor(parser)
		{
			this.parse = parser;
		}
	}

	function parseRule(input)
	{
		let token = input.expect(TokenTypes.identifier);
		if (!token)
			return null;

		let rule = _rules.get(token.str);
		if (!rule)
		{
			input.error(`Unknown rule: "${token.str}"`, token);
			return null;
		}

		let executor = rule.parse(input);
		if (!executor)
			return null;

		let node = new SyntaxNodeRule();
		node.executor = executor;
		return node;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////

	// RULES


	this.Rule("import", function(input)
	{
		// Env switch
		let importEnv = Environment.definitions;
		if (input.peek().is(TokenTypes.operators, '-')) // TODO: don't like this harded like this much.
		{
			let token = input.expect(TokenTypes.identifier);
			if (!token)
				return null;

			if ("instances".startsWith(token.str))
				importEnv = Environment.instances;
		}

		// TODO: not a big deal but this doesn't work if there's comments in the middle.
		let token = input.match(/\s*[\/a-zA-Z\d]+/g);
		if (!token)
		{
			input.error(`Invalid import path.`);
			return null;
		}

		let importPath = token.str.trim();
		if (!importPath)
		{
			input.error(`Encountered empty import path.`);
			return null;
		}

		return function(context)
		{
			let fullPath = isAbsolutePath(importPath) ? importPath :
				convertRelativePathToAbsolute(importPath, context.packagePath);

			fullPath = normalizePath(fullPath, importEnv); //`.${PackageExtension[importEnv]}`;

			if (importEnv === Environment.instances &&
				getEnvFromPackagePath(context.packagePath) !== Environment.instances)
			{
				console.error(`Definitions package '${context.packagePath}' cannot import instances package '${fullPath}'.`);
				return;
			}

			context.imports.push(fullPath);
		};
	});

	this.Rule("namespace", function(input)
	{
		let namespace = parseRecordName(input);
		if (!namespace)
			return null;

		// Do not allow env in namespace, env is determined by the package path.
		if (isFullRecordName(namespace))
		{
			input.error(`Namespace shouldn't include environment tag: ${namespace}`);
			let idx = namespace.indexOf(Punctuation.envDelim);
			namespace = namespace.substr(idx + 1);
		}

		return function(context)
		{
			context.scope = context.scope.clone();

			// Add trailing dot, so later it's easier to be combined with the object name.
			context.scope.namespace = namespace + Punctuation.nameDelim;
		};
	});

	this.Rule("using", function(input)
	{
		let namespaces = [];
		do//while ((input.peek()).is(TokenTypes.identifier))
		{
			let namespace = parseRecordName(input);
			if (!namespace)
				return null;

			// Same idea: adding trailing dot for convenience.
			namespaces.push(namespace + Punctuation.nameDelim);
		} 
		while(input.skip(TokenTypes.punctuation.itemDelim));

		if (namespaces.length === 0)
		{
			input.error(`Missing namespace following the using rule.`);
			return null;
		}

		return function(context)
		{
			context.scope = context.scope.clone();
			context.scope.usingNamespaces = context.scope.usingNamespaces.concat(namespaces);
		};
	});

	this.Rule("base", function(input)
	{
		let baseName = parseRecordName(input);
		if (!baseName)
			return null;

		return function(context)
		{
			context.defaultBase = baseName;
		};
	});


	/////////////////////////////////////////////////////////////////////////////////////////////////

	// CUSTOM VALUES

	class CustomValueType
	{
		constructor(parser, compiler)
		{
			this.parser = parser;
			this.compiler = compiler;
		}
	}

	class CustomValue
	{
		constructor() 							{}
		resolveBase(dependencies) 				{ throw ("not implemented"); }
		resolveReferences(scope, recordsMap) 	{ throw ("not implemented"); }
		assemble(base) 							{ throw ("not implemented"); }
		reproduce(originalValue)				{ throw ("not implemented"); }
	}


	// map
	class SyntaxNodeMap {constructor() { this.entries = []; }}

	class MapValue extends CustomValue
	{
		constructor()
		{
			super();
			this.entries = [];
		}

		resolveBase(dependencies)
		{
			for (let entry of this.entries)
			{
				let value = entry[1];
				if (value.constructor === Blueprint || 
					value.constructor === List || 
					value instanceof CustomValue)
				{
					if (!value.resolveBase(dependencies))
					{
						console.error(`Failed to resolve base for map value, key: ${entry[0]}, value: ${entry[1]}. Setting it to undefined`);
						entry[1] = undefined;
					}
				}
			}
			return true;
		}

		resolveReferences(scope, recordsMap)
		{
			for (let entry of this.entries)
			{
				let key = entry[0];
				let value = entry[1];

				if (key.constructor === ObjectReference)
					entry[0] = resolveReference(key.name, scope, recordsMap);

				if (value.constructor === ObjectReference)
					entry[1] = resolveReference(value.name, scope, recordsMap);

				if (value.constructor === Blueprint || 
					value.constructor === List || 
					value instanceof CustomValue)
				{
					value.resolveReference(scope, recordsMap);
				}
			}
		}

		assemble(base)
		{ 
			if (base && base.constructor !== MapValue)
				base = null;

			for (let entry of this.entries)
			{
				let key = entry[0];
				let value = entry[1];
				if (value.constructor === Blueprint || 
					value.constructor === List || 
					value instanceof CustomValue)
				{
					if (!base || !base.findEntry(key))
						value.assemble(null);
				}
			}

			if (base)
			{
				for (let entry of base.entries)
				{
					let key = entry[0];
					let myEntry = this.findEntry(key);

					if (myEntry)
					{
						let baseValue = entry[1];
						let myValue = myEntry[1];

						if (baseValue)
						{
							if ((myValue.constructor === baseValue.constructor &&
									(myValue.constructor === Blueprint || 
									myValue.constructor === List)) ||
								myValue instanceof CustomValue)
							{
								myValue.assemble(baseValue);	
							}
						}						
					}
					else
					{
						// insert into the beginning of the array (so all base entries appear first.)
						this.entries.unshift([...entry]);
					}					
				}
			}

			// TODO: consider checking duplicated keys here.		
		}
		
		reproduce(originalValue)
		{
			let originalMap = 
				(originalValue && originalValue.constructor === Map ? 
					originalValue : null);

			let map = (originalMap ? originalMap : new Map());

			for (let entry of this.entries)
			{
				let key = entry[0];
				let value = entry[1];

				if (value.constructor === Blueprint || 
					value.constructor === List || 
					value instanceof CustomValue)
				{
					let baseValue = originalMap.get(key);
					// TODO: The code here is correct, however currently List and Blueprint 
					// does not take originalValue in reproduce() function.
					map.set(key, value.reproduce(baseValue));
				}
				else
				{
					map.set(key, value);
				}
			}

			return map;
		}

		findEntry(key)
		{
			for (let entry of this.entries)
			{
				if (entry[0] === key)
					return entry;
			}

			return null;
		}
	}

	this.CustomValueType(parseMap, compileMap);

	function parseMap(input)
	{
		if (!input.peek().is(TokenTypes.identifier, "map"))
			return undefined;

		input.next(); // eat the "map"

		let mapNode = new SyntaxNodeMap();

		if (!input.expect(TokenTypes.punctuation, Punctuation.objectOpen))
			return null;

		while (!input.peek().is(TokenTypes.punctuation, Punctuation.objectClose))
		{
			let key = parseValue(input);
			if (!key)
				return null;

			if (!input.expect(TokenTypes.punctuation, Punctuation.assignment))
				return null;

			let value = parseValue(input);
			if (!value)
				return null;

			if (key.constructor === SyntaxNodeObject || 
				key.constructor === SyntaxNodeList ||
				key.constructor === SyntaxNodeCustomValue)
			{
				input.error(`Map key cannot be an object, list or custom value, use object reference instead. The parsing will continue but this key value pair is removed.`);
			}
			else
			{
				mapNode.entries.push([key, value]);
			}

			input.skip(TokenTypes.punctuation, Punctuation.itemDelim);
		}

		if (!input.expect(TokenTypes.punctuation, Punctuation.objectClose))
			return null;

		return mapNode;
	}

	function compileMap(mapNode)
	{
		let customValue = new MapValue();
		let dependencies = {};
		for (let entry of mapNode.entries)
		{
			let keyResult = compileValue(entry[0]);
			let valResult = compileValue(entry[1]);

			if (keyResult.value === undefined ||
				valResult.value === undefined)
			{
				console.error(`Unable to parse the key or value of a map entry. key type: ${entry[0].constructor}, value type: ${entry[1].constructor}`);
				continue;
			}

			customValue.entries.push([keyResult.value, valResult.value]);

			if (keyResult.dependencies)
				Object.assign(dependencies, keyResult.dependencies);

			if (valResult.dependencies)
				Object.assign(dependencies, valResult.dependencies);
		}

		return {customValue, dependencies};
	}















//  ██████╗ ██████╗ ███╗   ███╗██████╗ ██╗██╗     ██╗███╗   ██╗ ██████╗ 
// ██╔════╝██╔═══██╗████╗ ████║██╔══██╗██║██║     ██║████╗  ██║██╔════╝ 
// ██║     ██║   ██║██╔████╔██║██████╔╝██║██║     ██║██╔██╗ ██║██║  ███╗
// ██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██║██║     ██║██║╚██╗██║██║   ██║
// ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ██║███████╗██║██║ ╚████║╚██████╔╝
//  ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ 


	class Scope
	{
		constructor()
		{
			this.environment = Environment.definitions;
			this.namespace = ""; // with trailing name delimeter (dot)
			this.usingNamespaces = []; // also with trailing name delimeters			
		}

		clone()
		{
			let scope = new Scope();
			scope.environment = this.environment;
			scope.namespace = this.namespace;
			scope.usingNamespaces = this.usingNamespaces.slice();
			return scope;
		}
	}


	class PackageCompilingContext
	{
		constructor()
		{
			this.packagePath = "";
			this.defaultBase = "";
			this.scope = new Scope();
			this.records = [];
			this.imports = [];
		}
	}


	// return package info
	// all deps
	// all raw blobs
	function compilePackage(data, path)
	{
		let pkgNode = GLP.parse(data, _packageLexer, parsePackage);

		let context = new PackageCompilingContext();
		context.scope.environment = getEnvFromPackagePath(path);
		context.packagePath = path;		

		for (let entry of pkgNode.entries)
		{
			if (entry.constructor === SyntaxNodeProperty)
			{
				let env = context.scope.environment;
				let namespacedName = context.scope.namespace + entry.key;
				let fullName = `${env}${Punctuation.envDelim}${namespacedName}`;

				let {blueprint, dependencies} = compileBlueprint(entry.value, context.defaultBase);

				let record = new WorkingRecord();
				record.fullName = fullName;
				record.scope = context.scope;
				record.dependencies = dependencies;
				record.blueprint = blueprint;
				context.records.push(record);
			}
			else if (entry.constructor === SyntaxNodeRule)
			{
				entry.executor(context);
			}
		}

		return { records: context.records, imports: context.imports };
	}

	function compileBlueprint(objectNode, defaultBase)
	{
		let blueprint = new Blueprint();
		let dependencies = {};

		if (objectNode.base || defaultBase)
		{
			let baseName = (objectNode.base ? objectNode.base.name : defaultBase);
			if (!baseName)
				throw ("Somehow base name is empty");

			blueprint.baseName = baseName;
			dependencies[baseName] = null;	
		}	
		
		for (let prop of objectNode.properties)
		{
			let {value: propVal, dependencies: propDeps} = compileValue(prop.value);

			if (propVal !== undefined)
			{
				if (propVal.constructor === Blueprint)
				{
					blueprint.components[prop.key] = propVal;
				}
				else if (propVal.constructor === List)
				{
					blueprint.lists[prop.key] = propVal;
				}
				else if (propVal.constructor === ObjectReference)
				{
					blueprint.references[prop.key] = propVal;
				}
				else if (propVal instanceof CustomValue)
				{
					blueprint.customValues[prop.key] = propVal;
				}
				else
				{
					blueprint.values[prop.key] = propVal;
				}

				if (propDeps)
				{
					Object.assign(dependencies, propDeps);
				}
			}
			else
			{
				console.error(`Encounter unknown type blueprint property during compiling. key: "${prop.key}", value node:`, prop.value);
			}

			// switch (prop.value.constructor)	
			// {
			// 	case SyntaxNodeObject:
			// 	{
			// 		let result = compileBlueprint(prop.value, null);
			// 		blueprint.components[prop.key] = result.blueprint;
			// 		Object.assign(dependencies, result.dependencies);
			// 		break;
			// 	}
			// 	case SyntaxNodeObjectReference:
			// 	{
			// 		blueprint.references[prop.key] = prop.value.name;
			// 		break;
			// 	}
			// 	case SyntaxNodeList:
			// 	{
			// 		let result = compileList(prop.value);
			// 		blueprint.lists[prop.key] = result.list;
			// 		Object.assign(dependencies, result.dependencies);
			// 		break;
			// 	}
			// 	case SyntaxNodeString:
			// 	{
			// 		blueprint.values[prop.key] = prop.value.value;
			// 		break;
			// 	}
			// 	case SyntaxNodeNull:
			// 	{	blueprint.values[prop.key] = null;
			// 		break;
			// 	}
			// 	case SyntaxNodeExpression:
			// 	{
			// 		blueprint.values[prop.key] = evalExpresion(prop.value.value);
			// 		break;
			// 	}
			// }
		}

		return { blueprint, dependencies };
	}

	function compileList(listNode)
	{
		let list = new List();	
		let dependencies = {};	

		let itemType;
		for (let entry of listNode.entries)
		{
			switch (entry.constructor)
			{
			case SyntaxNodeObject:				itemType = ListItemType.blueprint; break;
			case SyntaxNodeObjectReference:		itemType = ListItemType.reference; break;
			case SyntaxNodeList:				itemType = ListItemType.list; break;
			case SyntaxNodeCustomValue:			itemType = ListItemType.customValue; break;
			case SyntaxNodeString:
			case SyntaxNodeExpression:			itemType = ListItemType.value; break;
			}

			if (itemType !== undefined)
				break;
		}

		for (let entry of listNode.entries)
		{
			let item;
			switch (entry.constructor)
			{
			case SyntaxNodeObject:
				if (itemType === ListItemType.blueprint)
				{
					let result = compileBlueprint(entry, null);
					item = result.blueprint;
					Object.assign(dependencies, result.dependencies);
				}
				break;

			case SyntaxNodeObjectReference:
				if (itemType === ListItemType.reference)
				{
					item = entry.name;
				}
				break;

			case SyntaxNodeList:
				if (itemType === ListItemType.list)
				{
					let result = compileList(entry);
					item = result.list;
					Object.assign(dependencies, result.dependencies);
				}
				break;

			case SyntaxNodeCustomValue:
				if (itemType === ListItemType.customValue)
				{
					let result = compileCustomValue(entry);
					item = result.customValue;
					Object.assign(dependencies, result.dependencies);
				}
				break;	

			case SyntaxNodeString:
				if (itemType === ListItemType.value)
				{
					item = entry.value;
				}
				break;

			case SyntaxNodeNull:
				if (itemType !== ListItemType.value)
				{
					item = null;
				}
				break;

			case SyntaxNodeExpression:
				if (itemType === ListItemType.value)
				{
					item = evalExpresion(entry.value);
				}
				break;
			}

			if (item !== undefined)
			{
				list.items.push(item);
			}
			else
			{
				console.error(`Incompatible list entry detected: "${entry}", list type: ${itemType}`);
			}
		}

		list.itemType = (itemType !== undefined ? itemType : ListItemType.value);
		return {list: list, dependencies: dependencies};
	}

	function compileCustomValue(customValNode)
	{
		let {customValue = null, dependencies = {}} = 
			customValNode.type.compiler(customValNode.value);

		return { customValue, dependencies };
	}

	function compileValue(valueNode)
	{
		switch (valueNode.constructor)	
		{
			case SyntaxNodeObject:
			{
				let result = compileBlueprint(valueNode, null);
				return { value: result.blueprint, dependencies: result.dependencies };
			}
			case SyntaxNodeObjectReference:
			{
				return { value: new ObjectReference(valueNode.name), dependencies: null };
			}
			case SyntaxNodeList:
			{
				let result = compileList(valueNode);
				return { value: result.list, dependencies: result.dependencies };
			}
			case SyntaxNodeCustomValue:
			{
				let result = compileCustomValue(valueNode);
				return { value: result.customValue, dependencies: result.dependencies };
			}
			case SyntaxNodeString:
			{
				return { value: valueNode.value, dependencies: null };
			}
			case SyntaxNodeNull:
			{	
				return { value: null, dependencies: null };
			}
			case SyntaxNodeExpression:
			{
				return { value: evalExpresion(valueNode.value), dependencies: null };
			}
			default:
			{
				return { value: undefined, dependencies: null };
			}
		}
	}

	function evalExpresion(expr)
	{
		/*jshint -W054 */
		try
		{
			let func = new Function(`return (${expr});`);
			let result = func();
			if (typeof result === 'number' && !Number.isFinite(result))
			{
				console.error(`Expression does not evaluate to a finite number (${result}): ${expr}`);
				return 0;
			}
			else
			{
				return result;
			}
		}
		catch (e)
		{
			console.error(`Invalid expression encountered during data compiling: ${expr}`);
			return 0;
		}
	}







// ██████╗ ██████╗  ██████╗  ██████╗███████╗███████╗███████╗██╗███╗   ██╗ ██████╗ 
// ██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██╔════╝██║████╗  ██║██╔════╝ 
// ██████╔╝██████╔╝██║   ██║██║     █████╗  ███████╗███████╗██║██╔██╗ ██║██║  ███╗
// ██╔═══╝ ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║╚════██║██║██║╚██╗██║██║   ██║
// ██║     ██║  ██║╚██████╔╝╚██████╗███████╗███████║███████║██║██║ ╚████║╚██████╔╝
// ╚═╝     ╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ 



	function processPackages(packages)
	{
		let processingRecords = [];

		for (let pkgPath in packages)
		{
			if (!_loadedPackages.has(pkgPath))
			{
				_loadedPackages.add(pkgPath);

				for (let record of packages[pkgPath])
				{
					if (_records.has(record.fullName))
					{
						console.error(`Records in different packages have duplicated name '${record.fullName}', the newly loaded one from ${pkgPath} is ignored.`);
					}
					else
					{
						processingRecords.push(record);
					}
				}
			}
		}

		return processRecords(processingRecords);
	}

	function processRecords(records)
	{
		let recordsMap = new Map();
		for (let record of records)
		{
			recordsMap.set(record.fullName, record);
		}

		// setup dependencies
		for (let record of records)
		{
			record.establishDependencies(recordsMap);
		}

		// sort dependencies
		let sortedRecords = sortRecordsByDependency(records);

		// Find the base of all blueprints with a baseName,
		// thus able to create a raw object to used in resolve ref phase.
		for (let record of sortedRecords)
		{
			record.resolveBase();
		}

		for (let record of sortedRecords)
		{
			record.resolveReferences(recordsMap);
		}

		// merge with base (recursively, owner to sub object)
		for (let record of sortedRecords)
		{
			record.assembleBlueprint();
		}

		let finalRecords = [];
		for (let record of sortedRecords)
		{
			let finalRecord = record.generateFinalRecord();
			if (finalRecord)
			{
				finalRecords.push(finalRecord);
				addRecord(finalRecord);
				//_records[record.fullName] = finalRecord;
			}
			else
			{
				console.error(`Record '${record.fullName}' did not load correctly.`);
			}
		}
		
		// Notify the objects of the loaded event.
		// And the object can add additional loading by returning a promise.
		let loadingPromises = [];
		for (let record of finalRecords)
		{
			let obj = record.object;
			let loaded = obj[ArchiveSymbol.loaded];
			if (loaded)
			{
				let result = loaded.call(obj, record);
				if (result !== undefined)
				{
					loadingPromises.push(Promise.resolve(result));
				}
			}
		}

		// At this point all records and pakcages are in place.
		// We are just waiting for the additional custom loading to finish,
		// which shouldn't be a problem should any additional loading call
		// occur or even dependant on the records loaded here, since they
		// are complete from record point of view. (with complete record info
		// and blueprint which can be used to extend child objects or used as
		// reference. The only caution is the record's object may have missing
		// content. That's why you need to wait for this promise to fullfill
		// before actually using the object for game purpose.)
		return Promise.all(loadingPromises);
	}

	class WorkingRecord
	{
		constructor()
		{
			this.fullName = "";
			this.scope = new Scope();
			this.dependencies = {}; // TODO: should be a map
			this.dependantCount = 0;
			this.blueprint = null;
			this.object = null;
		}

		establishDependencies(recordsMap)
		{
			for (let name in this.dependencies)
			{
				// Can be a class, a loaded record or a working record.
				let dependency = resolveDependency(name, this.scope, recordsMap);
				if (!dependency)
				{
					console.error(`Failed to resolve dependency '${name}' for '${this.fullName}'.`);
				}
				else if (dependency === this)
				{
					console.error(`Somehow the record has a dependency ('${name}') of itself: '${this.fullName}'.`);
				}
				else
				{
					this.dependencies[name] = dependency;

					// Initialize the working records count of how many other records are dependant on it.
					// during sorting this count will be decremented until it reaches 0 (if not, there's circular dependencies)
					if (dependency.constructor === WorkingRecord)
					{
						dependency.dependantCount++;
					}
				}
			}
		}

		resolveBase()
		{
			if (this.blueprint.resolveBase(this.dependencies))
			{
				this.object = this.blueprint.createObject();
			}
			else
			{
				console.error(`Failed to resolve the base of the blueprint for record: ${this.fullName}.`);
				this.blueprint = null; // remove the bad blueprint so it cannot contaminate the rest of the archive.
			}
		}

		resolveReferences(recordsMap)
		{
			if (this.blueprint)
				this.blueprint.resolveReferences(this.scope, recordsMap);
		}

		assembleBlueprint()
		{
			if (this.blueprint)
				this.blueprint.assemble(null);		
		}

		generateFinalRecord()
		{
			if (!this.blueprint)
				return null; // error already reported.

			this.blueprint.fillObject(this.object);

			let finalRecord = new Record();
			finalRecord.fullName = this.fullName;
			finalRecord.environment = this.scope.environment;
			finalRecord.blueprint = this.blueprint;
			finalRecord.object = this.object;
			return finalRecord;
		}
	}

	class Record
	{
		constructor()
		{
			this.fullName = "";
			this.environment = null;
			this.blueprint = null;
			this.object = null;
		}
	}

	var ListItemType =
	{
		value: 1,
		reference: 2,
		blueprint: 3,
		list: 4,
		customValue: 5,
	};

	class ObjectReference
	{
		constructor(name = "") { this.name = name; }
	}

	class List
	{
		constructor()
		{
			this.itemType = ListItemType.value;
			this.items = [];
		}

		resolveBase(dependencies)
		{
			if (this.itemType === ListItemType.blueprint ||
				this.itemType === ListItemType.list ||
				this.itemType === ListItemType.customValue)
			{
				for (let i = 0; i < this.items.length; i++)
				{
					// can have nulls
					let item = this.items[i];
					if (item)
					{
						if (!item.resolveBase(dependencies))
						{
							this.items[i] = null;
							console.error(`Blueprint item at ${i} failed to resolve base.`);
						}	
					}					
				}
			}
			return true;		
		}

		resolveReferences(scope, recordsMap)
		{
			if (this.itemType === ListItemType.blueprint ||
				this.itemType === ListItemType.list ||
				this.itemType === ListItemType.customValue)
			{
				for (let item of this.items)
				{
					if (item) item.resolveReferences(scope, recordsMap);
				}
			}
			else if (this.itemType === ListItemType.reference)
			{
				let newList = [];
				newList.length = this.items.length;

				for (var i = 0; i < this.items.length; i++) 
				{
					let item = this.items[i];
					newList[i] = item ? resolveReference(item, scope, recordsMap) : null;
				}
				this.items = newList;
			}
		}

		assemble(base)
		{
			if (base)
				throw ("list cannot have base.");

			if (this.itemType === ListItemType.blueprint ||
				this.itemType === ListItemType.list ||
				this.itemType === ListItemType.customValue)
			{
				for (let item of this.items)
				{
					// can have nulls
					if (item) item.assemble(null);
				}
			}
		}

		reproduce()
		{
			if (this.itemType === ListItemType.blueprint ||
				this.itemType === ListItemType.list ||
				this.itemType === ListItemType.customValue)
			{
				let newList = [];
				newList.length = this.items.length;

				for (var i = 0; i < this.items.length; i++) 
				{
					let item = this.items[i];
					newList[i] = item ? item.reproduce(null) : null;
				}
				return newList;
			}
			else
			{
				return this.items.slice();
			}
		}
	}

	class Blueprint
	{
		constructor()
		{
			this.baseName = "";
			this.base = null;
			this.cls = null;
			
			this.components = {};
			this.lists = {};
			this.customValues = {};
			this.references = {};
			this.values = {};
		}

		resolveBase(dependencies)
		{
			if (this.baseName)
			{
				// resolve
				let baseDep = dependencies[this.baseName] || null;

				if (!baseDep)
				{
					console.error(`Failed to resolve the base ${this.baseName}`);
					return false;
				}

				// a native class
				if (typeof baseDep === 'function')
				{
					this.cls = baseDep;
				}
				else
				{
					if (!baseDep.blueprint || !baseDep.blueprint.cls)
					{
						console.error(`Invalid base record ${this.baseName}: no blueprint, or blueprint has no cls: ${baseDep}`);
						return false;
					}

					this.base = baseDep.blueprint;
					this.cls = this.base.cls;
				}
			}
			// else
			// {
			// 	// TODO: this is not entirely correct, since if the blueprint doesn't have base, 
			// 	// it may still want to resolve base for its sub components or lists etc.
			// 	console.error(`Cannot resolve base, since the blueprint does not specify a base name.`);
			// 	return false;
			// }

			// TODO: can we merge the code of these three?
			// but may run into the problem of deoptimizing.
			for (let key in this.components)
			{
				if (!this.components[key].resolveBase(dependencies))
				{
					delete this.components[key]; // remove the bad component. error already reported.
				}
			}

			// Self lists
			for (let key in this.lists)
			{
				if (!this.lists[key].resolveBase(dependencies))
				{
					delete this.lists[key];
				}
			}

			for (let key in this.customValues)
			{
				if (!this.customValues[key].resolveBase(dependencies))
				{
					console.error(`Failed to resolve base for custom value with key ${key}.`);
					delete this.customValues[key];
				}
			}

			return true;
		}

		resolveReferences(scope, recordsMap)
		{
			for (let component of Object.values(this.components))
			{
				component.resolveReferences(scope, recordsMap);
			}

			for (let list of Object.values(this.lists))
			{
				list.resolveReferences(scope, recordsMap);
			}

			for (let customVal of Object.values(this.customValues))
			{
				customVal.resolveReferences(scope, recordsMap);
			}

			for (let key in this.references)
			{
				// Can be null, but the error has been reported by resolveReference().
				let referencedObject = resolveReference(this.references[key].name, scope, recordsMap);

				if (this.values.hasOwnProperty(key))
					console.error(`Duplicated property key '${key}'' between references and values: ${this.values[key]}.`);

				this.values[key] = referencedObject;
			}
			// TODO: should we clear the references array?
		}		

		assemble(base)
		{
			// TODO: maybe the assemble function here can determine based on has cls or not,
			// and the outside will always pass in base when see fit.
			if (this.cls && base)
				throw ("The blueprint has its own base, should not receive a base from outside.");

			this.base = base = (base || this.base);

			// Self components
			for (let key in this.components)
			{
				let component = this.components[key];
				if (component.cls || !base || !base.components.hasOwnProperty(key))
				{
					component.assemble(null);
				}
			}

			// Self lists
			for (let key in this.lists)
			{
				this.lists[key].assemble(null);
			}

			// Self custom values
			for (let key in this.customValues)
			{
				if (!base || !base.customValues.hasOwnProperty(key))
				{
					this.customValues[key].assemble(null);	
				}
			}

			// Merge in base
			if (base)
			{
				this.cls = base.cls;

				for (let key in base.components)
				{
					let baseComponent = base.components[key];
					let myComponent = this.components[key];
					if (myComponent)
					{
						if (!myComponent.cls)
						{
							myComponent.assemble(baseComponent);
						}
					}
					else
					{
						this.components[key] = baseComponent;
					}
				}

				for (let key in base.customValues)
				{
					let baseCustomVal = base.customValues[key];
					let myCustomVal = this.customValues[key];
					if (myCustomVal)
					{
						myCustomVal.assemble(baseCustomVal);
					}
					else
					{
						this.customValues[key] = baseCustomVal;
					}
				}				

				for (let key in base.lists)
				{
					if (!this.lists.hasOwnProperty(key))
					{
						this.lists[key] = base.lists[key];
					}
				}

				for (let key in base.values)
				{
					if (!this.values.hasOwnProperty(key))
					{
						this.values[key] = base.values[key];
					}
				}
			}
		}

		reproduce()
		{
			if (!this.cls)
			{
				console.error(`The blueprint cannot reproduce since its class is null, this blueprint can only be used to fill the existing object which must be supplied by the owner's constructor.`);
				return null;
			}

			return this.fillObject(this.createObject());
		}

		createObject()
		{
			return new this.cls();
		}

		fillObject(obj)
		{
			// TODO: maybe reproduce() and fillObject() should always take an original value as parameter,
			// so inside each reproduce function it can check if the original value is valid and check types etc.

			for (let key in this.components)
			{
				let component = this.components[key];
				let originalValue = obj[key];
				if (component.cls)
				{
					let newObject = component.reproduce();

					if (originalValue === null || originalValue === undefined ||
						(typeof originalValue === 'object' &&
						newObject instanceof originalValue.constructor))
					{
						obj[key] = newObject;
					}
					else
					{
						console.error(`Failed to assign property '${key}' in object '${this.baseName?this.baseName:obj.constructor}': Type mismatch between original (${originalValue}) and assigning value (${newObject}).`);
					}					
				}
				else
				{
					if (originalValue && typeof originalValue === 'object' && !Array.isArray(originalValue))
					{
						obj[key] = component.fillObject(originalValue);
					}
					else
					{
						console.error(`Failed to assign property '${key}' in object '${this.baseName?this.baseName:obj.constructor}': The original value (${originalValue}) is not of object type when attempting to partially override.`);
					}
				}
			}

			for (let key in this.lists)
			{
				let originalValue = obj[key];
				if (originalValue === null || originalValue === undefined ||
					Array.isArray(originalValue))
				{
					obj[key] = this.lists[key].reproduce();	
				}
				else
				{
					console.error(`Failed to assign property '${key}' in object '${this.baseName?this.baseName:obj.constructor}': The original value (${originalValue}) is not an array and cannot be assigned to array value.`);
				}
			}

			for (let key in this.customValues)
			{
				let originalValue = obj[key];
				obj[key] = this.customValues[key].reproduce(originalValue);
			}

			for (let key in this.values)
			{
				let originalValue = obj[key];
				let newValue = this.values[key];
				let originalType = typeof originalValue;
				let newType = typeof newValue;
				let isTypeMatch = (originalType === newType || originalValue === undefined);
				let isObjTypeMatch = (originalType !== 'object' || !originalValue || !newValue ||
					newValue instanceof originalValue.constructor);

				if (isTypeMatch && isObjTypeMatch)
				{
					obj[key] = newValue;
				}
				else
				{
					console.error(`Failed to assign property '${key}' in object '${this.baseName?this.baseName:obj.constructor}': Type mismatch between original (${originalValue}) and assigning value (${newValue}).`);
				}
			}

			return obj;
		}
	}

	function sortRecordsByDependency(records)
	{
		let sortedRecords = [];
		let recordsWithNoDependant = [];

		for (let record of records)
		{
			if (record.dependantCount <= 0)
			{
				recordsWithNoDependant.push(record);
			}
		}

		while (recordsWithNoDependant.length > 0)
		{
			let record = recordsWithNoDependant.pop();
			sortedRecords.push(record);

			for (let name in record.dependencies)
			{
				let dependency = record.dependencies[name];
				if (dependency && dependency.constructor === WorkingRecord)
				{
					if (dependency.dependantCount <= 0)
						throw ("incorrect dependant count.");

					if (--dependency.dependantCount <= 0)
					{
						recordsWithNoDependant.push(dependency);
					}
				}
			}
		}

		let cyclicDependencies = [];
		for (let record of records)
		{
			if (record.dependantCount > 0)
			{
				cyclicDependencies.push(record);
			}
		}

		if (cyclicDependencies.length > 0)
		{
			console.error("Detected following records with cyclic dependencies (they will not be processed or loaded, and their dependants may be loaded with errors as well:");
			for (let record of cyclicDependencies)
			{
				console.error(`    - ${record.fullName}`);
			}
		}
		
		// The sorting algorithm puts the dependant records earlier than their dependencies.
		// Now reverse since we need to process the dependencies first.
		sortedRecords.reverse();
		return sortedRecords;
	}

	function resolveDependency(name, scope, recordsMap)
	{
		let cls = resolveNativeClass(name);
		let rec = resolveReferencedRecord(name, scope, recordsMap, !cls);

		if (cls && rec)
			console.error(`Ambigious base dependency: can be a class "${cls}" or a record ${rec.fullName}. Returning the calss.`);

		return cls ? cls : rec;
	}

	function resolveReference(refName, scope, recordsMap)
	{
		let rec = resolveReferencedRecord(refName, scope, recordsMap, true);
		return rec ? rec.object : null;
	}

	function resolveNativeClass(className)
	{
		/*jshint -W054 */
		let lookUpFunc = new Function(`try { return ${className}; } catch (e) { return null; }`);
		let cls = lookUpFunc();

		if (!cls)
		{
			//console.error(`Class not found: ${className}`);
			return null;
		}

		if (typeof cls !== 'function')
		{
			console.error(`Class ${className} is not a function. It is ${cls}.`);
			return null;
		}

		if (cls.length !== 0)
		{
			console.error(`Class ${className} is a function but has parameters`);
			return null;
		}

		return cls;
	}

	function resolveReferencedRecord(refName, scope, recordsMap, logNotFound)
	{
		let namespacedNames = [refName];
		if (!isFullRecordName(refName))
		{
			for (let namespace of scope.usingNamespaces)
			{
				namespacedNames.push(`${namespace}${refName}`);
			}	
		}		

		let envs = [scope.environment];
		if (scope.environment === Environment.instances)
			envs.push(Environment.definitions);

		let fullNames = [];
		for (let name of namespacedNames)
		{
			if (isFullRecordName(name))
			{
				// Definitions cannot reference instances
				if (scope.environment === Environment.definitions &&
					name.startsWith(Environment.instances))
				{
					console.error(`Definition cannot reference instance: ${name}`);
				}
				else
				{
					fullNames.push(name);	
				}
			}
			else
			{
				for (let env of envs)
				{
					fullNames.push(`${env}${Punctuation.envDelim}${name}`);
				}
			}
		}

		let foundRecs = [];
		for (let fullName of fullNames)
		{
			let rec = _records.get(fullName) || recordsMap.get(fullName);
			if (rec) foundRecs.push(rec);
		}

		if (foundRecs.length === 0)
		{
			if (logNotFound)
				console.error(`Reference to '${refName}' not found. Attempted names include: ${namespacedNames} in ${envs}`);
			
			return null;
		}
		else
		{
			if (foundRecs.length > 1)
			{
				console.error(`Found ambigious named (${refName}) records: ${foundRecs.map(rec => rec.fullName)}. Using ${foundRecs[0].fullName}`);
			}
			return foundRecs[0];
		}
	}

	function isFullRecordName(name)
	{
		if (!(typeof name === 'string'))
			throw(`The record name is not a string: ${name}`);

		return (name.indexOf(Punctuation.envDelim) >= 0);
	}

})(this);





























































var ArchiveOld = new (function(global) 
{
	var objects = {};
	var records = {};
	var namespaces = {};
	var currentNamespace = null;
	var globalRegs = {};
	var defCharCode = 46; // '.'
	var instCharCode = 35; // '#'

	this.init = function()
	{
		compileRecords();
		clearGlobals();
	};

	this.find = function(name)
	{
		return objects[name] || null;
	};

	this.get = function(name)
	{
		var obj = objects[name];
		if (!obj)
			throw ("object with name " + name  + " does not exist.");

		return obj;
	};

	this.getAll = function(namespace)
	{
		var list = namespaces[namespace];
		if (list === undefined)
			throw ("namespace does not exist! " + namespace);

		return list;
	};

	this.create = function(base, props, name)
	{
		if (props === undefined)
		{
			props = {};
		}

		let baseName = "";
		if (typeof base === 'string')
		{
			baseName = trimNamespace(base);
		}
		else if (typeof base === 'function')
		{
			baseName = base.name;
		}
		else if (typeof base === 'object')
		{
			base = this.getName(base);
			baseName = trimNamespace(base);
		}

		if (!base || !baseName)
		{
			console.error(`Invalid base "${base}" or its name "${baseName}", failed to create new object.`);
			return null;
		}

		props.$base = base;

		if (name === undefined)
		{
			name = findUniqueObjectName(baseName);
		}

		load(name, props, true);
		compileRecords();
		return this.get(name);
	};

	this.delete = function(obj)
	{
		var name = obj.name();
		if (objects[name] !== obj)
			throw ("The given object is not in the archive.");

		delete objects[name];
		removeFromNamespaces(name, obj);
	};

	this.select = function(where)
	{
		var found = [];
		for (var name in objects)
		{
			var obj = objects[name];
			if (where(obj))
				found.push(obj);
		}
		return found;
	};

	this.getName = function(obj)
	{
		if (obj.$name)
		{
			return obj.$name.substr(1);
		}
		else
		{
			return "";
		}
	};

	this.isInstance = function(obj)
	{
		return obj.$name && obj.$name.charCodeAt(0) === instCharCode;
	};

	var Record = Class(
	{
		constructor: function(name, props, namespace, isInstance)
		{
			this.name = name;
			this.props = props;
			this.namespace = namespace;
			this.isInstance = isInstance;
			this.base = null;
			this.index = -1;
			this.obj = null;
			this.binders = [];
			this.finished = false;
		}
	});

	var Namespace = Class(
	{
		constructor: function()
		{
			this.prefix = "";
			this.uses = [];
		}
	});

	var BaseInfo = Class(
	{
		constructor: function()
		{
			this.rec = null;
			this.obj = null;
			this.cls = null;
		},

		derive: function()
		{
			if (this.rec)
			{
				this.obj = this.rec.obj;
				if (!this.obj)
					throw ("record object not constructed.");
			}

			if (this.obj)
			{
				this.cls = this.obj.constructor;
			}
		}
	});

	var CustomValue = Class(
	{
		constructor : function(raw, parser)
		{
			this.raw = raw;
			this.parser = parser || null;
		}
	});

	var Binder = Class(
	{
		constructor: function(target, source, sourceRec, sourceIsBase)
		{
			this.target = target;
			this.source = source;
			this.sourceRec = sourceRec || null;
			this.sourceIsBase = sourceIsBase || false;
		},

		canBind: function()
		{
			return !this.sourceRec || this.sourceRec.finished;
		},

		bind: function(record)
		{
			var source = this.source;
			var target = this.target;

			if (this.sourceIsBase && target.hasOwnProperty('$baseObj'))
			{
				target.$baseObj = source;
			}	

			var keys = Object.keys(source);
			for (var i = keys.length - 1; i >= 0; i--) 
			{
				var key = keys[i];
				
				// TODO: should use defineProperty() to hide these?
				if (key[0] === '$')
					continue;

				// TODO: it may open can of worms if we start allowing missing properties (accidently having bunch of plain objects!)
				// but how do we make this explicit?
				var isPlainObject = target.constructor === Object;
				if (!target.hasOwnProperty(key) && !isPlainObject)
					throw ("property does not match: " + key);

				var	value = source[key];

				var newValue = bindProperty(target, key, value, record, this.sourceIsBase);

				if (newValue !== undefined)
					target[key] = newValue;
			}
		}
	});

	var CustomBinder = Class(Binder,
	{
		constructor : function(target, source, parser)
		{
			CustomBinder.$super.call(this, target, source, null);
			this.parser = parser;
		},

		bind: function(record)
		{
			var value = this.parser(this, record);
			// parse failed (likely due to required objects not fully loaded), insert back and try later.
			if (value === undefined)
			{
				record.binders.push(this);
			}
			else // the raw source value has been parsed into value, now insert a normal binder to bind parsed value to the target.
			{
				record.binders.push(new Binder(this.target, value, null, false));
			}
		},

		// this is used by the parser to retrieve objects in the archive.
		// Only the fully loaded object (record marked as finished) will be returned, otherwise return null
		// currently there's no way to know whether the object simply doesn't exist (in that case will throw) or still being loaded.
		resolveObject: function(name, record)
		{
			var fullName = resolveName(name, record.namespace);
			var rec = records[fullName];
			if (rec)
			{
				if (rec.finished)
				{
					return rec.obj;
				}
				else
				{
					this.sourceRec = rec;
					return null;
				}
			}
			else
			{
				var obj = objects[fullName];
				if (!obj)
					throw ("wrong name.");

				return obj;
			}
		}
	});

	function load(name, props, isInstance)
	{
		if (currentNamespace !== null)
			name = currentNamespace.prefix + name;

		if (!name)
			throw ("invalid name.");

		if (records[name] || objects[name])
			throw ("data with name " + name + " already exists!");

		if (!props.$base)
			throw ("the record missing base: " + name);

		records[name] = new Record(name, props, currentNamespace, isInstance);
	}

	function compileRecords()
	{
		if (currentNamespace !== null)
			throw ("namespace not closed!");

		var queue = [];

		// insert all records into working queue
		var record;
		for (var name in records)
		{
			record = records[name];
			if (record.index < 0)
			{
				enqueueRecord(record, queue);
			}
		}

		// construct objects in queue
		var len = queue.length;
		for (var i = 0; i < len; i++) 
		{
			initializeRecord(queue[i]);
		}

		// extend objects in queue
		var deferredQueue = new Queue(); 
		for (i = 0; i < len; i++)  
		{
			record = queue[i];
			if (!processRecord(record))
				deferredQueue.enqueue(record);
		}

		// deferred queue due to dependency
		while (!deferredQueue.isEmpty())
		{
			var numDeferred = deferredQueue.len;
			for (i = 0; i < numDeferred; i++) 
			{
				record = deferredQueue.dequeue();
				if (!processRecord(record))
					deferredQueue.enqueue(record);
			}

			if (numDeferred === deferredQueue.len)
				throw ("no record in deferred queue was processed, likely circular dependency.");
		}

		// finalize, move into final objects map, clear records
		for (i = 0; i < len; i++) 
		{
			record = queue[i];
			var obj = record.obj;
			objects[record.name] = obj;
			addToNamespaces(record.name, obj);
			obj.init();
		}

		records = {};
	}

	function enqueueRecord(record, queue) 
	{
		if (queue.indexOf(record) >= 0)
			throw ("The record is already in the queue.");

		record.base = resolveBase(record.props.$base, record.namespace);
		var baseRec = record.base.rec;
		if (baseRec && baseRec.index < 0)
		{
			enqueueRecord(baseRec, queue);
		}

		record.index = queue.length;
		queue.push(record);
	}

	function initializeRecord(record) 
	{
		record.base.derive();
		var baseCls = record.base.cls;

		record.obj = constructObject(baseCls);

		if (record.obj.$name === undefined)
			throw ("archived object must have a $name property.");

		var prefix = String.fromCharCode(record.isInstance ? instCharCode : defCharCode);
		record.obj.$name = prefix + record.name;
		/*
		if (typeof baseCls !== 'function')
			throw ("base must be a constructor function");

		if (baseCls.length !== 0)
			throw ("constructor must have no arguments");

		record.obj = new baseCls();*/

		//delete record.props.$base;
	}

	function processRecord(record) 
	{
		var binders = record.binders;
		if (binders.length === 0)
		{
			binders.push(new Binder(record.obj, record.props, null, false));

			if (record.base.obj)
			{
				binders.push(new Binder(record.obj, record.base.obj, record.base.rec, true));
			}
		}

		while (binders.length > 0)
		{
			var binder = binders[binders.length - 1];
			if (!binder.canBind())
				return false;

			binders.pop();

			binder.bind(record);
			//bindObject(binder.target, binder.source, record);
		}

		record.finished = true;
		return true;
	}

	function constructObject(cls)
	{
		if (typeof cls !== 'function')
			throw ("base must be a constructor function");

		if (cls.length !== 0 && cls !== Object)
			throw ("constructor must have no arguments");

		return new cls();
	}

	// function bindObject(target, source, record)
	// {
	// 	var keys = Object.keys(source);
	// 	for (var i = keys.length - 1; i >= 0; i--) 
	// 	{
	// 		var key = keys[i];
			
	// 		// TODO: should use defineProperty() to hide these?
	// 		if (key[0] === '$')
	// 			continue;

	// 		if (!target.hasOwnProperty(key))
	// 			throw ("property does not match: " + key);

	// 		var	value = source[key];

	// 		var newValue = bindProperty(target, key, value, record);

	// 		if (newValue !== undefined)
	// 			target[key] = newValue;


/*
			// Array
			if (Array.isArray(value))
			{
				var targetList = target[key];
				if (targetList === null)
				{
					targetList = [];
					target[key] = targetList;
				}
				else if (Array.isArray(targetList))
				{
					targetList.length = 0;
				}
				else
				{
					throw ("unmatching type");
				}

				bindList(targetList, value, record);
			}
			// String
			else if (typeof value === 'string')
			{
				// Reference
				if (value[0] === '@')
				{
					var ref = resolveReference(value.substr(1), record.namespace);
					target[key] = ref;
				}
				else
				{
					target[key] = value;
				}
			}
			// Object
			// TODO: handle regex (proper?)
			else if (typeof value === 'object' && value !== null && value.constructor != RegExp)
			{
				var targetObj = target[key];
				if (typeof targetObj !== 'object')
				{
					throw ("type mismatch");
				}

				// Reference to the archived object
				if (value.$name && value.$name.length > 0)
				{
					target[key] = value;
				}
				else
				{
					var sourceObj = value;
					var sourceBase = null;
					var sourceBaseRec = null;
					var sourceCls = null;

					// Plain object that defines the $base as property
					if (value.$base)
					{
						// TODO: verify it's a plain object?
						var baseInfo = resolveBase(value.$base, record.namespace);
						baseInfo.derive();
	
						sourceBase = baseInfo.obj;
						sourceBaseRec = baseInfo.rec;
						sourceCls = baseInfo.cls;
					}
					// Normally constructed object
					else
					{
						sourceCls = value.constructor;
					}
					
					if (targetObj === null || (targetObj.constructor !== sourceCls && sourceCls !== Object))
					{
						targetObj = constructObject(sourceCls);
						target[key] = targetObj;
					}

					record.binders.push(new Binder(targetObj, sourceObj, null));

					if (sourceBase)
						record.binders.push(new Binder(targetObj, sourceBase, sourceBaseRec));
				}
			}
			// Simple value
			else
			{
				// TODO: type check needed?
				target[key] = value;
			}*/
	// 	};
	// };

	function bindList(target, source, record, fromBase)
	{
		var l = source.length;
		target.length = l;
		for (var i = 0; i < l; i++) 
		{			
			target[i] = bindProperty(null, "", source[i], record, fromBase);
		}		
	}

	function bindProperty(target, key, sourceValue, record, fromBase)
	{
		var newValue;

		// Array
		if (Array.isArray(sourceValue))
		{
			var targetList = target ? target[key] : null;
			if (targetList === null || targetList === undefined)
			{
				newValue = targetList = [];
			}
			else if (Array.isArray(targetList))
			{
				targetList.length = 0;
			}
			else
			{
				throw ("unmatching type");
			}

			bindList(targetList, sourceValue, record, fromBase);
		}
		// String
		else if (typeof sourceValue === 'string')
		{
			// Reference
			if (sourceValue[0] === '@')
			{
				newValue = resolveReference(sourceValue.substr(1), record.namespace);
			}
			else
			{
				newValue = sourceValue;
			}
		}
		// Object
		// TODO: handle regex (proper?)
		else if (typeof sourceValue === 'object' && sourceValue !== null && sourceValue.constructor !== RegExp)
		{
			var targetObj = target ? target[key] : null;
			if (targetObj === undefined)
				targetObj = null;

			if (typeof targetObj !== 'object')
			{
				throw ("type mismatch");
			}

			// Reference to the archived object
			if ((sourceValue.$name && sourceValue.$name.length > 0) ||  // reference to archived object
				(sourceValue.constructor !== Object && sourceValue.$canBeSubObject !== true)) // source can only be referenced, and not merged as sub object (must opt in!)
			{
				newValue = sourceValue;
			}
			else if (sourceValue.constructor === CustomValue)
			{
				var parser = sourceValue.parser;
				if (parser === null && targetObj)
					parser = targetObj.parse;

				if (!parser)
					throw ("No parser found for custom value.");

				record.binders.push(new CustomBinder(targetObj, sourceValue.raw, parser));
			}
			else
			{
				var sourceObj = sourceValue;
				var sourceBase = null;
				var sourceBaseRec = null;
				var sourceCls = null;

				// Plain object that defines the $base as property
				if (sourceValue.$base)
				{
					// TODO: verify it's a plain object?
					var baseInfo = resolveBase(sourceValue.$base, record.namespace);
					baseInfo.derive();

					sourceBase = baseInfo.obj;
					sourceBaseRec = baseInfo.rec;
					sourceCls = baseInfo.cls;
				}
				// Normally constructed object
				else
				{
					sourceCls = sourceValue.constructor;
				}
				
				// TODO: (already done, confirm validity) maybe should reconstruct the targetObject as long as the sourceBase is defined.
				// TODO: perhaps, it should be an error, if the sourceCls === Object, but the targetObj === null,
				// this way we can make assigning plain object (as map) an explicit thing.
				// but if the target is already a plain object, we should not care...
				if (targetObj === null || sourceBase || (targetObj.constructor !== sourceCls && sourceCls !== Object))
				{
					targetObj = constructObject(sourceCls);
					newValue = targetObj;
				}

				record.binders.push(new Binder(targetObj, sourceObj, null, (fromBase && sourceBase === null)));

				if (sourceBase)
					record.binders.push(new Binder(targetObj, sourceBase, sourceBaseRec, true));
			}
		}
		// Simple value
		else
		{
			// TODO: type check needed?
			newValue = sourceValue;
		}

		return newValue;
	}

	function resolveBase(base, namespace)
	{
		var info = new BaseInfo();
		if (typeof base === 'string')
		{
			var baseName = resolveName(base, namespace);
			var baseRec = records[baseName];
			if (baseRec)
			{
				info.rec = baseRec;
			}
			else
			{
				info.obj = objects[baseName];
				if (!info.obj) 
					throw ("wrong name");
			}
		}
		// TODO: check if base is a function (constructor) or object
		else if (base)
		{
			info.cls = base;
		}
		else
		{
			throw ("unresolvable base.");
		}

		return info;
	}

	function resolveReference(name, namespace)
	{
		var fullName = resolveName(name, namespace);
		var rec = records[fullName];
		return rec ? rec.obj : objects[fullName];
	}

	function resolveName(name, namespace)
	{
		if (records[name] || objects[name])
			return name;

		if (namespace)
		{
			var namespaceName = namespace.prefix + name;
			if (records[namespaceName] || objects[namespaceName])
				return namespaceName;

			var uses = namespace.uses;
			var l = uses.length;
			for (var i = 0; i < l; i++) 
			{
				var useName = uses[i] + name;
				if (records[useName] || objects[useName])
					return useName;
			}	
		}

		throw ("cannot resolve name: " + name);
	}

	function addToNamespaces(name, obj)
	{
		var delim = -1;
		while (true)
		{
			delim = name.indexOf('.', delim + 1);
			if (delim < 0)
				break;

			var namespace = name.substring(0, delim);
			var list = namespaces[namespace];
			if (list === undefined)
			{
				list = [];
				namespaces[namespace] = list;
			}

			list.push(obj);
		}
	}

	function removeFromNamespaces(name, obj)
	{
		var delim = -1;
		while (true)
		{
			delim = name.indexOf('.', delim + 1);
			if (delim < 0)
				break;

			var namespace = name.substring(0, delim);
			var list = namespaces[namespace];
			if (list !== undefined)
			{
				var index = list.indexOf(obj);
				if (index >= 0)
					list.splice(index, 1);
			}
		}
	}

	function beginNamespace(prefix)
	{
		if (currentNamespace !== null)
			throw ("namespace not closed!");

		currentNamespace = new Namespace();

		if (prefix)
			currentNamespace.prefix = prefix.trim() + '.';
	}

	function endNamespace()
	{
		if (currentNamespace === null)
			throw ("no namespace opened.");

		currentNamespace = null;
	}

	function useNamespace(prefix)
	{
		if (currentNamespace === null)
			throw ("no namespace opened.");

		if (!prefix)
			throw ("not valid namespace.");

		currentNamespace.uses.push(prefix + '.');
	}

	function useNamespaces(prefixs)
	{
		var tokens = prefixs.split(/\s+/);
		for (var i = 0; i < tokens.length; i++) 
		{
			useNamespace(tokens[i]);
		}
	}

	function trimNamespace(name)
	{
		let idx = name.lastIndexOf('.');
		if (idx >= 0)
		{
			return name.substr(idx + 1);
		}
	}

	function findUniqueObjectName(baseName)
	{
		let max = 0x1000000000000; // 2^48
		let numBytes = 6;
		let bytes = new Uint8Array(numBytes);
		let lowerMask = 0x00FFFFFF;
		let mask = 0xFF;
		let uniqueName = "";

		do
		{
			let rand = Math.floor(Math.random() * max);
			let lower = rand & lowerMask;
			let higher = Math.floor((rand - lower) / (lowerMask + 1));

			// little endian, higher to lower bits.
			for (let i = numBytes - 1; i >= numBytes/2; i--) 
			{
				bytes[i] = (lower & mask);
				lower >>= 8; // Shift one byte
			}

			for (let i = numBytes/2 - 1; i >= 0; i--) 
			{
				bytes[i] = (higher & mask);
				higher >>= 8; // Shift one byte
			}

			let byteStr = String.fromCharCode.apply(null, bytes);
			let encoding = btoa(byteStr);
			uniqueName = `${baseName}[${encoding}]`;

		} while(records[uniqueName] || objects[uniqueName]);

		return uniqueName;
	}

	function loadDefinition(name, props)
	{
		load(name, props, false);
	}

	function loadInstance(name, props)
	{
		//props.isInstance = true;
		load(name, props, true);
	}

	function customValue(raw, parser)
	{
		return new CustomValue(raw, parser);
	}

	function loadCollection(namespace, uses, collection, isInstance)
	{
		beginNamespace(namespace);

		if (uses)
			useNamespaces(uses);

		for (var name in collection)
		{
			load(name, collection[name], isInstance);
		}

		endNamespace();
	}

	function loadDefinitions(namespace, uses, collection)
	{
		if (arguments.length === 0)
		{
			throw ("missing arguments!");
		}
		else if (arguments.length === 1)
		{
			loadCollection("", "", arguments[0], false);
		}
		else if (arguments.length === 2)
		{
			loadCollection(namespace, "", arguments[1], false);
		}
		else
		{
			loadCollection(namespace, uses, collection, false);
		}
	}

	function loadInstances(namespace, uses, collection)
	{
		if (arguments.length === 0)
		{
			throw ("missing arguments!");
		}
		else if (arguments.length === 1)
		{
			loadCollection("", "", arguments[0], true);
		}
		else if (arguments.length === 2)
		{
			loadCollection(namespace, "", arguments[1], true);
		}
		else
		{
			loadCollection(namespace, uses, collection, true);
		}
	}

	globalFunc("$def", loadDefinition);
	globalFunc("$inst", loadInstance);
	globalFunc("$defs", loadDefinitions);
	globalFunc("$insts", loadInstances);
	globalFunc("$begin", beginNamespace);
	globalFunc("$end", endNamespace);
	globalFunc("$use", useNamespace);
	globalFunc("$v", customValue);

	function globalFunc(name, func)
	{
		if (globalRegs[name] || global[name])
			throw ("the global function with the same name has been registered! " + name);

		globalRegs[name] = func;
		global[name] = func;
	}

	function clearGlobals()
	{
		for (var name in globalRegs)
		{
			delete global[name];
		}

		globalRegs = {};
	}

})(this);


