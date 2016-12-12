/*global Archive: true*/
/*exported Archive*/
var Archive = new (function(global) 
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


