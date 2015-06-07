var UniqueId =
{
	_usedIds : {},

	New : function(base)
	{
		var next = 0;
		if (this._usedIds.hasOwnProperty(base))
			next = this._usedIds[base] + 1;
		this._usedIds[base] = next;
		return base + "_" + next;
	}
};

var isArray = function(obj)
{
	return obj.length !== undefined && (obj.length === 0 || obj[0] !== undefined);
};

var $handler = function(obj, name)
{
	return obj[name].bind(obj);
};

var $callback = function(obj, name, handler)
{
	if (!handler)
	{
		if (!obj.$callbacks)
			return dummyHandler;

		handler = obj.$callbacks[name];
		return handler ? handler : dummyHandler;
	}
	else
	{
		if (!obj.$callbacks)
			obj.$callbacks = {};

		obj.$callbacks[name] = handler;
		return handler;
	}

	function dummyHandler() {};
}

var Cloner = new (function(global)
{
	this.clone = function(source)
	{
		var cloned = this.construct(source);
		this.extend(cloned, source);
		return cloned;
	};

	this.construct = function(source)
	{
		var cloned
		if (isArray(source))
		{
			cloned = [];
			if (Object.getPrototypeOf(cloned) !== Object.getPrototypeOf(source))
				throw("clone list that is not created from [] is not supported!");
		}
		else
		{
			cloned = Object.create(Object.getPrototypeOf(source));
		}

		//cloned.constructor = source.constructor;
		return cloned;
	};

	this.extend = function(target, override, shallow)
	{
		shallow = shallow || false;
		var props = Object.getOwnPropertyNames(override);
		var l = props.length;
		for (var i = 0; i < l; i++) 
		{
			var key = props[i];
			var value = override[key];

			// TOOD: optimization allow shallow copy
			if (!shallow && typeof value === 'object' && value !== null && !value.hasOwnProperty("$ref"))
			{
				target[key] = this.clone(value);
			}
			else
			{
				target[key] = value;
			}			
		};
		return target;
	};

	this.extendBatch = function(shallow, objects)
	{
		var l = arguments.length;
		if (l < 3)
			throw ("too few arguments!");

		var target = arguments[1];
		for (var i = 2; i < l; i++) 
		{
			this.extend(target, arguments[i], shallow);
		};
		return target;
	};

	this.replaceReferences = function (data, refs)
	{
		for (var name in data)
		{
			var value = data[name];
			if (typeof value === "string")
			{
				if (value.length > 0 && value[0] === '$')
				{
					var ref = refs[value.substr(1)];
					if (ref === undefined)
						throw ("cannot find reference to " + value.substr(1));
					data[name] = ref;
				}
			}
			else if (typeof value === "object")
			{
				this.replaceReferences(value, refs);
			}
		}
	};

})(this);


var LatentUpdater = function(owner, updateFunc)
{
	this.owner = owner;
	this.updateFunc = updateFunc;
	this.running = false;
	this.updating = false;
	this.stopped = false;
	this.waiting = false;

	this.start = function()
	{
		if (this.running)
			throw ("already running. Multiple call to start()!");

		if (this.updating || this.stopped || this.waiting)
			throw ("in bad state!");

		this.running = true;
		resume.call(this);
	};

	this.updateFinished = function(shouldStop)
	{
		if (!this.updating)
			throw ("cannot finish if not updating!");

		this.updating = false;
		this.stopped = shouldStop || false;

		if (this.waiting)
		{
			this.waiting = false;
			resume.call(this);
		}
	};

	function resume() 
	{
		while (!this.stopped)
		{
			this.updateFunc.call(this.owner);
			if (this.updating)
			{
				this.waiting = true;
				return;
			}
		};

		this.stopped = false;
		this.updating = false;
		this.waiting = false;
		this.running = false;
	};
};
