var Components = new (function(global)
{
	var compInfo = {};
	this._comps = {};

	this.get = function(name)
	{
		var comp = this._comps[name];
		if (!comp)
		{
			var cls = getClass(name);
			comp = new cls();
			comp.name = name;
			var base = compInfo[name].base;
			comp.base = base ? this.get(base) : null;
			this._comps[name] = comp;
		}
		return comp;
	};

	var BaseComponent = Class(
	{
		onAttached : function(entity, name)
		{
			this.entity = entity;
			this.name = name;
			this._bindEvents();
		},

		_bindEvents : function()
		{

		}
	});

	function getClass(name)
	{
		var info = compInfo[name];
		if (!info)
			throw ("cannot find component " + name);

		if (!info.cls)
		{
			var base = info.base;
			if (base)
			{
				var superClass = getClass(base);
				info.cls = Class(superClass, info.body);
			}
			else
			{
				info.cls = Class(BaseComponent, info.body);
			}
			// TODO: may want to delete body
		}

		return info.cls;
	};

	global.Component = function(name, base, body)
	{
		var info = (arguments.length <= 2) ? 
			{ base : null, body : base } : 
			{ base : base, body : body };

		compInfo[name] = info;
	};

	global.Entity = Class(
	{
		constructor : function()
		{
			this._components = [];
		},

		attachComponent : function(type, name)
		{
			var component = new getClass(type)();
			var generatedName = this._generateComponentName(arguments.length <= 1 ? type : name);
			this._components.push(component);
			component.onAttached(this, generatedName);
		},

		_generateComponentName : function(name)
		{
			var foundMatching = false;

			for (var i = this._components.length - 1; i >= 0; i--) 
			{
				if (this._components[i].name == name)
				{
					foundMatching = true;
					break;
				}
			};

			if (!foundMatching)
				return name;

			var base = this._nameToBaseAndNumber(name)[0];
			var number = 0;

			for (var i = this._components.length - 1; i >= 0; i--) 
			{
				var other = this._nameToBaseAndNumber(this._components[i].name);
				if (base == other[0])
					number = Math.max(number, other[1])
			};

			return base + '_' + (number + 1);
		},

		_nameToBaseAndNumber : function(name)
		{
			var delimIdx = name.lastIndexOf('_');
			var base = delimIdx >= 0 ? name.substr(0, delimIdx) : name;
			var number = delimIdx >= 0 ? parseInt(name.substr(delimIdx+1)) : 0;
			if (number == NaN)
				number = 0;

			return [base, number];
		},

		_broadcastEvent : function(event, args)
		{

		}

	});

})(this);

var Definition = new (function(global)
{
	var registered = {};
	var defs = {};

	var OBJECT = "object";
	var NUMBER = "number";
	var LENGTH = "length";

	function isString(obj) { return toString.apply(obj) === "[object String]"; };
	function isMap(obj) { return (obj && typeof obj === OBJECT && !(typeof obj.length === NUMBER && !(obj.propertyIsEnumerable(LENGTH)))); };

	this.register = function(settings, defs)
	{
		if (arguments.length < 2) { defs = settings; settings = {}; }

		var base = settings.base || null;
		var group = settings.group || "";


		for (var name in defs)
		{
			var raw = defs[name];

			if (!raw.hasOwnProperty("$base"))
				raw.$base = base;

			if (group != "")
				name = group + "." + name;

			raw.$name = name;
			registered[name] = raw;
		}
	};

	this.get = function(name)
	{
		this.get = getDefFast;
		initialize();
		return getDefFast.call(this, name); // call this.get(name)? look like recursive but not?
	};

	function initialize()
	{
		for (var name in registered) 
		{
			createDef(registered[name]);
		}

		for (var name in registered)
		{
			extendDefData(defs[name], registered[name]);
		}

		for (var name in defs)
		{
			var def = defs[name];
			if (def.init !== undefined)
				def.init();
		}

		registered = null;
	};

	function createDef(data)
	{
		var name = data.$name;
		if (data.$created)
			return defs[name];

		if (defs.hasOwnProperty(name))
			throw ("duplicated definition with name " + name);

		var def = null;
		var baseDef = null;
		var base = data.$base;

		if (typeof base === 'string')
		{
			var baseData = registered[base];
			if (baseData === undefined)
				throw ("cannot find base def " + base);

			baseDef = createDef(baseData);
			def = Cloner.construct(baseDef);
		}
		else if (typeof base === 'function')
		{
			def = new base();
		}
		else
		{
			throw ("unsupported base type " + base);
		}

		defs[name] = def;
		def.$ref = true;
		data.$baseDef = baseDef;
		data.$created = true;
	};

	function extendDefData(def, data)
	{
		if (data.$extended)
			return;

		var baseDef = data.$baseDef
		if (baseDef !== null)
		{
			extendDefData(baseDef, registered[data.$base]);
			Cloner.extend(def, baseDef);
		}

		Cloner.replaceReferences(data, defs);

		delete data["$base"];
		delete data["$baseDef"];
		delete data["$created"];

		Cloner.extend(def, data);
		data.$extended = true;
	};

	function getDefFast(name)
	{
		var def = defs[name];
		if (def === undefined)
			throw ("cannot find definition: " + name);

		return def;
	};

})(this);
