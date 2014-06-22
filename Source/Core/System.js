var $p = function()
{
	//var proto = [];

	var has = function(name)
	{
		return this.indexOf(name) !== -1;
	};

	/*
	constr = function(data)
	{
		Array.prototype.push.apply(this, data);
	};

	constr.prototype = proto;
*/
	return function()
	{
		var paramDef = [];
		Array.prototype.push.apply(paramDef, arguments);
		
		paramDef.has = has;
		paramDef.$ref = true;
		return paramDef;
	};
}();

var Params = Class(
{
	constructor : function(list)
	{
		this.list = list;
		this.length = list.length;
	},

	set : function(name, value)
	{
		if (!this.list.has(name))
			throw ("parameter named " + name + " cannot be found!");

		this[name] = value;
	},

	batch : function(values)
	{
		if (values.length != this.list.length)
			throw ("number of parameters does not match, required: " + this.list.length + ", given: " + values.length);

		for (var i = 0; i < values.length; i++) 
		{
			this[this.list[i]] = values[i];
		};
	},

	get : function(name)
	{
		return this[name];
	},

	isa : function(value, type)
	{
		return true;
	},

	equals : function(params)
	{
		var l = this.list.length;
		for (var i = 0; i < l; i++)
		{
			var name = this.list[i];
			if (this[name] !== params[name])
				return false;
		}

		return true;
	},

	attachTo : function(context)
	{
		var l = this.list.length;
		for (var i = 0; i < l; i++)
		{
			var name = this.list[i];
			if (context.hasOwnProperty(name))
				throw ("cannot attach property " + name + " already in context");

			context[name] = this[name];
		}		
	},

	detachFrom : function(context)
	{
		var l = this.list.length;
		for (var i = 0; i < l; i++)
		{
			var name = this.list[i];
			delete context[name];
		}
	},

	toString : function()
	{
		var list = this.list;
		var str = "";
		for (var i = 0; i < list.length; i++)
		{
			if (i > 0) str += ",";
			var valueStr = this[list[i]].toString();
			if (valueStr[0] === "[")
				throw ("incorrect value string format, make sure you implement the toString()" + valueStr);

			str += valueStr;
		};

		return str;
	}
});

var ConstParams = Class(Params, 
{
	constructor : function(list, values)
	{
		ConstParams.$super.call(this, list);
		ConstParams.$superp.batch.call(this, values);
	},

	set : function(name, value)
	{
		throw ("cannot modify constant parameters");
	},

	batch : function(values)
	{
		throw ("cannot modify constant parameters");
	}
});

var $e = function(exprStr)
{
	return exprStr;
};

var Evaluator = new (function(global)
{
	this.eval = function(value, context)
	{
		with(context)
		{
			return eval(value);
		}
	};

	this.evalList = function(values, context)
	{
		var results = [];
		for (var i = 0; i < values.length; i++) 
		{
			results.push(this.eval(values[i], context));
		};
		return results;
	};

})(this);