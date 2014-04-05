var Core =
{
	_hasInit : false,
	//_components : {},
	//_componentClasses : {},

	_cardSets : [],
	_cards : {},

	_definitions : {},

	init : function()
	{
		this._loadCardSets();
		this._initDefinitions();
		this._hasInit = true;
	},

	/*

	Component : function(name, base, cls)
	{
		if (name in this._components)
			throw ("Component " + name + " already registered!");

		var superCls = CardComponent;
		var superComp = null;
		if (arguments.length <= 2)
		{
			cls = base;
		}
		else
		{
			superCls = this._componentClasses[base];
			superComp = this._components[base];
			if (superCls === undefined)
				throw ("Component " + name + "'s " + "super class " + base + " cannot be found!");
		}

		var compCls = Class(superCls, cls);
		var comp = new compCls();
		comp.name = name;
		comp.superComponent = superComp;
		this._components[name] = comp;
		this._componentClasses[name] = compCls;
	},

	getComponent : function(name)
	{
		if (!this._components.hasOwnProperty(name))
			throw ("Component " + name + " not found!");

		return this._components[name];
	},
*/
	CardSet : function(set)
	{
		this._cardSets.push(set);
	},

	getCard : function(name)
	{
		if (!this._cards.hasOwnProperty(name))
			throw ("cannot find card definition " + name);

		return this._cards[name];
	},

	findCard : function(name)
	{
		if (!this._cards.hasOwnProperty(name))
			return null;

		return this._cards[name];
	},

	hasCard : function(name)
	{
		return this._cards.hasOwnProperty(name);
	},

	getCards : function(comp)
	{
		if (arguments.length <= 0)
			comp = "";

		var cards = [];
		for (var name in this._cards)
		{
			if (this._cards.hasOwnProperty(name))
			{
				var card = this._cards[name];

				if (comp !== "" && !card.has(comp))
					continue;

				cards.push(this._cards[name]);
			}
		}
		return cards;
	},

	addCard : function(name, def)
	{
		if (this._cards.hasOwnProperty(name))
			throw ("trying to add card with duplicated name "+name);

		var card =  new CardDefinition(def);
		card.name = name;
		this._cards[name] = card;
		card.postLoad();
	},

	_loadCardSets : function()
	{
		for (var i = 0; i < this._cardSets.length; i++)
			this._loadCardSet(this._cardSets[i]);

		var cards = this.getCards();
		for (i = 0; i < cards.length; i++)
			cards[i].postLoad();
	},

	_loadCardSet : function(set)
	{
		var base = set.Base;

		for (var name in set)
		{
			if (name === "Base")
				continue;

			if (set.hasOwnProperty(name))
			{
				if (this._cards.hasOwnProperty(name))
					throw ("multiple cards have same name "+name);

				var def = this._extendCardDef(base, set[name]);
				var card =  new CardDefinition(def);
				card.name = name;
				this._cards[name] = card;
			}
		}
	},

	_extendCardDef : function(base, child)
	{
		if (base === undefined)
			return child;

		var def = $.extend({}, base, child);

		if (base.comps && child.comps)
			def.comps = base.comps + " " + child.comps;

		if (base.tags && child.tags)
			def.tags = base.tags + ", " + child.tags;

		// TODO: need real card extension handling that can trace base of the base
		def._base = base;

		return def;
	},


	Definitions : function(base, defs)
	{
		for (var name in defs)
		{
			var def = defs[name];
			if (!def.hasOwnProperty("_base"))
				def._base = base;
			if (this._definitions.hasOwnProperty(name))
				throw ("duplicated definition name: " + name);
			this._definitions[name] = def;
		}
	},

	getDef : function(name)
	{
		if (!this._definitions.hasOwnProperty(name))
			throw ("cannot find definition: " + name);

		return this._definitions[name];
	},

	findDef : function(name)
	{
		if (!this._definitions.hasOwnProperty(name))
			return null;

		return this._definitions[name];
	},

	addDef : function(name, def)
	{
		if (this._hasInit)
			throw ("add definition after initialized is not supported yet!");

		if (this._definitions.hasOwnProperty(name))
			throw ("duplicated definition name: " + name);

		this._definitions[name] = def;
	},

	_initDefinitions : function()
	{
		var defs = {};
		for (var name in this._definitions)
		{
			var def = this._extendDefinition(name);
			var defObj = new def._base();
			$.extend(defObj, def);
			defObj.name = name;
			delete defObj._base;
			delete defObj._extended;
			defs[name] = defObj;
		}

		this._definitions = defs;

		for (var i = defs.length - 1; i >= 0; i--) 
		{
			defs[i].init();
		};
	},

	_extendDefinition : function(name, children)
	{
		var def = this._definitions[name];
		if (def === undefined)
			throw ("cannot find definition: " + name);

		if (def._extended)
			return def;

		if (typeof def._base == 'string')
		{
			children = children || {};
			if (children[name])
				throw ("circular definition hierarchy: " + name);
			children[name] = true;
			var base = this._extendDefinition(def._base, children);
			def = $.extend({}, base, def);
			def._base = base._base;
			def._extended = true;
			this._definitions[name] = def;
			return def;
		}
		else
		{
			if (typeof def._base != 'function')
				throw ("definition must have constructor or another definition as base, given:" + this._base);

			def._extended = true;
			return def;
		}
	}
};

var Events =
{
	bind : function(source, name, handler, target)
	{
		if (source.__eventHub === undefined)
			source.__eventHub = this._newEventHub();

		if (source.__eventHub.triggering > 0)
			source.__eventHub.pendings.push([true, name, handler, target]);
		else
			this._doBind(source, name, handler, target);
	},

	unbind : function(source, name, target)
	{
		if (source.__eventHub === undefined)
			return;

		if (source.__eventHub.triggering > 0)
			source.__eventHub.pendings.push([false, name, target]);
		else
			this._doUnbind(source, name, target);
	},

	delegate : function(source, name, handler, target)
	{
		if (target.__eventHub === undefined)
			target.__eventHub = this._newEventHub();

		var delegate = target.__eventHub.delegates[name];
		if (delegate !== undefined)
		{
			if (delegate[0] === source && delegate[1] === handler)
				return;
			this.unbind(delegate[0], name, target);
		}
		this.bind(source, name, handler, target);
		target.__eventHub.delegates[name] = [source, handler];
	},

	trigger : function(source, name, data)
	{
		if (source.__eventHub === undefined)
			return;

		var binding = source.__eventHub.bindings[name];
		if (binding === undefined)
			return;

		source.__eventHub.triggering++;

		var args = arguments.length > 3 ? 
			Array.prototype.slice.call(arguments, 2) :
			(arguments.length === 3 ? [data] : []);

		args.push(source);

		for (var i = 0; i < binding.length; i++)
		{
			binding[i][0].apply(binding[i][1], args);
		}

		source.__eventHub.triggering--;

		if (source.__eventHub.triggering <= 0)
			this._flushPendingBindings(source);
	},

	_newEventHub : function()
	{
		var hub = {};
		hub.bindings = {};
		hub.delegates = {};
		hub.pendings = [];
		hub.triggering = 0;
		return hub;
	},

	_doBind : function(source, name, handler, target)
	{
		target = target || source;

		var hub = source.__eventHub;
		var binding = hub.bindings[name];
		if (binding === undefined)
		{
			binding = [];
			hub.bindings[name] = binding;
		}

		for (var i = 0; i < binding.length; i++)
		{
			var existing = binding[i];
			if (existing[0] === handler && existing[1] === target)
				return;
		}

		binding.push([handler, target]);
	},

	_doUnbind : function(source, name, target)
	{
		target = target || source;

		var hub = source.__eventHub;
		var binding = hub.bindings[name];
		if (binding === undefined)
			return;

		for (var i = binding.length - 1; i >= 0; i--)
		{
			var existing = binding[i];
			if (existing[1] === target)
				binding.splice(i, 1);
		}
	},

	_flushPendingBindings : function(source)
	{
		var pendings = source.__eventHub.pendings;
		var l = pendings.length;
		for (var i = 0; i < l; i++)
		{
			var p = pendings[i];
			if (p[0])
				this._doBind(source, p[1], p[2], p[3]);
			else
				this._doUnbind(source, p[1], p[2]);
		}
		source.__eventHub.pendings.length = 0;
	}
};