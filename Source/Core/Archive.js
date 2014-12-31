
var Archive = new (function(global)
{
	var objects = {};
	var records = {};
	var currentNamespace = null;

	var Record = Class(
	{
		constructor: function(name, props, namespace)
		{
			this.name = name;
			this.props = props;
			this.namespace = namespace;
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

	var Binder = Class(
	{
		constructor: function(target, source, sourceRec)
		{
			this.target = target;
			this.source = source;
			this.sourceRec = sourceRec || null;
		}
	});

	this.init = function()
	{
		compileRecords();
		clearGlobals();
	};

	function load(name, props)
	{
		if (currentNamespace !== null)
			name = currentNamespace.prefix + name;

		if (!name)
			throw ("invalid name.");

		if (records[name] || objects[name])
			throw ("data with name " + name + " already exists!");

		records[name] = new Record(name, props, currentNamespace);
	};

	function compileRecords()
	{
		if (currentNamespace !== null)
			throw ("namespace not closed!");

		var recordQueue = [];

		// insert all records into working queue
		for (var name in records)
		{
			var record = records[name];
			if (record.index < 0)
			{
				enqueueRecord(record, queue);
			}
		};

		// construct objects in queue
		var len = queue.length;
		for (var i = 0; i < len; i++) 
		{
			initializeRecord(queue[i]);
		};

		// extend objects in queue
		var deferredQueue = new Queue();
		for (var i = 0; i < len; i++) 
		{
			var record = queue[i];
			if (!processRecord(record))
				deferredQueue.enqueue(record);
		};

		// deferred queue due to dependency
		while (!deferredQueue.isEmpty())
		{
			var numDeferred = deferredQueue.len;
			for (var i = 0; i < numDeferred; i++) 
			{
				var record = deferredQueue.dequeue();
				if (!processRecord(record))
					deferredQueue.enqueue(record);
			};

			if (numDeferred === deferredQueue.len)
				throw ("no record in deferred queue was processed, likely circular dependency.");
		}

		// finalize, move into final objects map, clear records
		for (var i = 0; i < len; i++) 
		{
			var record = queue[i];
			objects[record.name] = record.obj;
		};

		records = {};
	};

	function enqueueRecord(record, queue) 
	{
		record.base = resolveBase(record.props.$base, record.namespace);
		var baseRec = record.base.rec;
		if (baseRec && baseRec.index < 0)
		{
			enqueueRecord(baseRec, queue);
		}

		queue.push(record);
	};

	function initializeRecord(record) 
	{
		record.base.derive();
		var baseCls = record.base.cls;

		record.obj = constructObject(baseCls);

		if (record.obj.$name === undefined)
			throw ("archived object must have a $name property.");

		record.obj.$name = record.name;
		/*
		if (typeof baseCls !== 'function')
			throw ("base must be a constructor function");

		if (baseCls.length !== 0)
			throw ("constructor must have no arguments");

		record.obj = new baseCls();*/

		//delete record.props.$base;
	};

	function processRecord(record) 
	{
		var binders = record.binders;
		if (binders.length === 0)
		{
			binders.push(new Binder(record.obj, record.props, null));

			if (record.base.obj)
				binders.push(new Binder(record.obj, record.base.obj, record.base.rec));
		}

		while (binders.length > 0)
		{
			var binder = binders[binders.length - 1]
			if (binder.sourceRec && !binder.sourceRec.finished)
				return false;

			binders.pop();

			bindObject(binder.target, binder.source, record);
		}

		record.finished = true;
		return true;
	};

	function constructObject(cls)
	{
		if (typeof cls !== 'function')
			throw ("base must be a constructor function");

		if (cls.length !== 0)
			throw ("constructor must have no arguments");

		return new cls();
	};

	function bindObject(target, source, record)
	{
		var keys = Object.keys(source);
		for (var i = keys.length - 1; i >= 0; i--) 
		{
			// TODO: should use defineProperty() to hide these?
			if (key[0] === '$')
				continue;

			var key = keys[i];
			if (!target.hasOwnProperty(key))
				throw ("property does not match: " + key);

			var value = source[key];

			var newValue = bindProperty(target, key, value, record);

			if (newValue !== undefined)
				target[key] = newValue;


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
		};
	};

	function bindList(target, source, record)
	{
		var l = source.length;
		target.length = l;
		for (var i = 0; i < l; i++) 
		{			
			target[i] = bindProperty(null, "", source[i], record);
		};		
	};

	function bindProperty(target, key, sourceValue, record)
	{
		var newValue = undefined;

		// Array
		if (Array.isArray(sourceValue))
		{
			var targetList = target ? target[key] : null;
			if (targetList === null)
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

			bindList(targetList, sourceValue, record);
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
		else if (typeof sourceValue === 'object' && sourceValue !== null && sourceValue.constructor != RegExp)
		{
			var targetObj = target ? target[key] : null;
			if (typeof targetObj !== 'object')
			{
				throw ("type mismatch");
			}

			// Reference to the archived object
			if (sourceValue.$name && sourceValue.$name.length > 0)
			{
				newValue = sourceValue;
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
				
				if (targetObj === null || (targetObj.constructor !== sourceCls && sourceCls !== Object))
				{
					targetObj = constructObject(sourceCls);
					newValue = targetObj;
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
			newValue = sourceValue;
		}

		return newValue;
	};

	function resolveBase(base, namespace)
	{
		var info = new BaseInfo();
		if (typeof base === 'string')
		{
			var baseName = resolveName(base, record.namespace);
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
		else
		{
			info.cls = base;
		}

		return info;
	};

	function resolveReference(name, namespace)
	{
		var fullName = resolveName(name, namespace);
		var rec = records[fullName];
		return rec ? rec.obj : objects[fullName];
	};

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
			};	
		}

		throw ("cannot resolve name: " + name);
	};

	function beginNamespace(prefix)
	{
		if (currentNamespace !== null)
			throw ("namespace not closed!");

		currentNamespace = new Namespace();

		if (prefix)
			currentNamespace.prefix = prefix + '.';
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

	globalFunc("$obj", load);
	globalFunc("$begin", beginNamespace);
	globalFunc("$end", endNamespace);
	globalFunc("$use", useNamespace);

	var globalRegs = {};

	function globalFunc(name, func)
	{
		if (globalRegs[name] || global[name])
			throw ("the global function with the same name has been registered! " + name);

		globalRegs[name] = func;
		global[name] = func;
	};

	function clearGlobals()
	{
		for (var name in globalRegs)
		{
			delete global[name];
		}

		globalRegs = {};
	};

})(this);


