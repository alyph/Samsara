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

	this.extend = function(target, override)
	{
		var props = Object.getOwnPropertyNames(override);
		var l = props.length;
		for (var i = 0; i < l; i++) 
		{
			var key = props[i];
			var value = override[key];

			// TOOD: optimization allow shallow copy
			if (typeof value === 'object' && value !== null && !value.hasOwnProperty("$ref"))
			{
				target[key] = this.clone(value);
			}
			else
			{
				target[key] = value;
			}			
		};
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

