
var NewUI = new (function(global)
{
	var numLoadingHtml = 0;
	var readyCallback = null;
	var templates = {};
	var behaviors = {};
	var properties = {};

	var ATTR_BEHAVIORS = "$behaviors";

	this.registerTemplates = function(htmlUrl)
	{
		numLoadingHtml++;
		$.ajax(
		{ 
			url: htmlUrl,
			cache: false,
			success: function(data)
			{
				var tempRoot = document.createElement("div");
				tempRoot.innerHTML = data;

				var numTemplates = tempRoot.children.length;
				for (var i = 0; i < numTemplates; i++) 
				{
					var template = tempRoot.children[i];
					registerTemplate(template);
				};

				numLoadingHtml--;
				checkReady();
			} 
		});

		return this;
	};

	this.ready = function(callback)	
	{
		if (readyCallback !== null)
			throw ("Already has ready callback!");

		readyCallback = callback;
		checkReady();
	};

	this.Behavior = function(name, config)
	{
		if (behaviors[name] !== undefined)
			throw (`The behavior with name ${name} is already registered.`);

		var beh = new Behavior(name, config);
		behaviors[name] = beh;
		return beh;
	};

	function checkReady()
	{
		if (readyCallback !== null)
		{
			var isReady = numLoadingHtml <= 0;
			if (isReady)
			{
				var callback = readyCallback;
				readyCallback = null;
				callback();
			}
		}
	};


// .########.########.##.....##.########..##..........###....########.########
// ....##....##.......###...###.##.....##.##.........##.##......##....##......
// ....##....##.......####.####.##.....##.##........##...##.....##....##......
// ....##....######...##.###.##.########..##.......##.....##....##....######..
// ....##....##.......##.....##.##........##.......#########....##....##......
// ....##....##.......##.....##.##........##.......##.....##....##....##......
// ....##....########.##.....##.##........########.##.....##....##....########

	var Template = Class(
	{
		constructor : function(templateElement)
		{
			this.name = templateElement.id;
			this.element = templateElement;
			this.proto = Object.create(CustomElement.prototype);			
			this.elementInfos = [];
			this.dependencies = null;
			//this.bindingInfos = [];
			//this.behaviors = [];
			
			var template = this;
			this.proto.createdCallback = function()
			{
				CustomElement.apply(this);
				this.setup(template);
			};

			this.proto.detachedCallback = function()
			{
				this.bind(null);
			};

			this.collectElementInfo(this.element, []);
			this.collectChildrenInfos(this.element.content.children, []);
		},

		collectElementInfo : function(element, path)
		{
			var isRoot = (path.length === 0);
			var isCustomElement = isRoot || (element.tagName.indexOf('-') >= 0);

			var info = new ElementInfo();

			if (element.children.length === 0 && !isRoot)
			{
				var func = this.compileBinding(element.textContent);
				if (func !== null)
				{
					info.bindingFunc = func;
					element.textContent = "";
				}
			}

			if (element.hasAttribute(ATTR_BEHAVIORS))
			{
				var behaviorNames = element.getAttribute(ATTR_BEHAVIORS).trim().split(/\s+/);
				element.removeAttribute(ATTR_BEHAVIORS);
				for (var i = 0; i < behaviorNames.length; i++) 
				{
					var behavior = findBehavior(behaviorNames[i]);
					if (behavior)
					{
						info.behaviors.push(behavior);
					}
					else
					{
						console.error("Cannot find behavior: " + behaviorNames[i]);
					}
				};
			}

			// TODO: do not allow non-custom element to have custom attributes
			// Although here you can't check element.template to know if it's
			// custom or not, must check if it has - in the tag
			var attrs = element.attributes;
			for (var i = 0; i < attrs.length; i++) 
			{
				var attr = attrs[i];
				if (attr.name.length > 0 && attr.name[0] === '$')
				{
					var key = attr.name.substr(1).trim().replace(/[\s-_]+([a-zA-Z])/g,
						function(match, p1) { return p1.toUpperCase(); });
					
					var prop = findProperty(key);
					if (prop)
					{
						if (prop.readonly || !isCustomElement)
						{
							element.dataset[key] = attr.value;
						}
						else
						{
							var propInfo = new ElementPropertyInfo();
							propInfo.property = prop;
							propInfo.value = prop.parse(attr.value);
							info.properties.push(propInfo);
						}
					}
					else
					{
						console.error("Cannot find property: " + key);
					}

					element.removeAttribute(attr.name);
				}
			};

			// if (info.properties.length > 0 && !isCustomElement)
			// {
			// 	console.error("Cannot apply custom properties to non-custom element: " + element.tagName);
			// 	info.properties.length = 0; 
			// }

			if (info.bindingFunc !== null || info.behaviors.length > 0 || info.properties.length > 0)
			{
				info.path = path.concat();
				this.elementInfos.push(info);

				var elementName = element.tagName.toLowerCase();
				if (isCustomElement && !isRoot && templates[elementName] === undefined)
				{
					if (this.dependencies === null)
						this.dependencies = {};

					this.dependencies[elementName] = true;
				}
			}
		},

		collectChildrenInfos : function(children, path)
		{
			var l = children.length;
			for (var i = 0; i < l; i++) 
			{
				path.push(i);

				var element = children[i];

				this.collectElementInfo(element, path);

				if (element.children.length > 0)
				{
					this.collectChildrenInfos(element.children, path);	
				}

				path.pop();
			};
		},

		compileBinding : function(expression)
		{
			var input = expression.trim();
			var reg = /\{\{(.*)\}\}/g;
			var last = 0;
			var tokens = [];
			var numBindings = 0;
			var result;
			while ((result = reg.exec(input)) !== null) 
			{
				if (reg.index > last)
				{
					tokens.push(JSON.stringify(input.substring(last, result.index)));
				}

				last = reg.lastIndex;

				var bindingExpr = result[1].trim();
				if (bindingExpr === ".") bindingExpr = "arguments[0]"; // TODO: potential optimization.
				tokens.push("(" + bindingExpr + ")");
				numBindings++;
			}

			if (numBindings === 0)
			{
				return null;
			}

			if (last < input.length)
			{
				tokens.push(JSON.stringify(input.substring(last)));
			}

			var body = "if (data === null) return null; " +
					   "with (data) { return " + tokens.join(" + ") + "; };";

			return new Function("data", body);
		}
	});	

	var ElementInfo = Class(
	{
		constructor : function ElementInfo()
		{
			this.path = null;
			this.bindingFunc = null;
			this.behaviors = [];
			this.properties = [];
		}
	});

	var ElementPropertyInfo = Class(
	{
		constructor : function ElementPropertyInfo()
		{
			this.property = null;
			this.value = null;
		}
	});

	var pendingTemplates = {};

	function registerTemplate(templateElement)
	{
		var template = new Template(templateElement);
		if (template.dependencies)
		{
			var dependencies = Object.keys(template.dependencies);
			for (var i = 0; i < dependencies.length; i++) 
			{
				var dependency = dependencies[i];
				if (pendingTemplates[dependency] === undefined)
					pendingTemplates[dependency] = [];

				pendingTemplates[dependency].push(template);
			};
		}
		else
		{
			finishRegisteringTemplate(template);
		}
	    
	    return template;
	};

	function finishRegisteringTemplate(template)
	{
		var name = template.name;

		if (template[name])
		{
			Console.error("Duplicated template registeration: " + name);
			return;
		}

		document.registerElement(name, { prototype: template.proto });
		templates[name] = template;

		var pendings = pendingTemplates[name];
		if (pendings !== undefined)
		{
			for (var i = 0; i < pendings.length; i++) 
			{
				var pending = pendings[i];

				if (pending.dependencies)
				{
					delete pending.dependencies[name];
					if (Object.keys(pending.dependencies).length === 0)
						pending.dependencies = null;
				}

				if (!pending.dependencies)
					finishRegisteringTemplate(pending);
			};

			delete pendingTemplates[name];
		}
	};

	function isCustomElement(element)
	{
		return element.template && true;
	};

// .########.##.......########.##.....##.########.##....##.########
// .##.......##.......##.......###...###.##.......###...##....##...
// .##.......##.......##.......####.####.##.......####..##....##...
// .######...##.......######...##.###.##.######...##.##.##....##...
// .##.......##.......##.......##.....##.##.......##..####....##...
// .##.......##.......##.......##.....##.##.......##...###....##...
// .########.########.########.##.....##.########.##....##....##...

	function CustomElement() 
	{
		this.template = null;
		this.bindings = [];
		this.dataContext = null;
		this.dataObserver = null;
		this.properties = null;
	};
	var customElemProto = CustomElement.prototype = Object.create(HTMLElement.prototype);
	customElemProto.constructor = CustomElement;

	customElemProto.setup = function(template)
	{
		this.template = template;
		var templateElem = template.element;

		// TODO: do not create shadow root if the content is empty?
        // Add the template content to the shadow root
		var clonedContent = document.importNode(templateElem.content, true);
        var shadowRoot = this.createShadowRoot();
        shadowRoot.appendChild(clonedContent);

        // TODO: simplify
        var numClasses = templateElem.classList.length;
        for (var i = 0; i < numClasses; i++) 
        {
        	this.classList.add(templateElem.classList.item(i));
        };

        // TODO: may want to inline to improve perf.
        //attachBehaviors(this);
        //initBindings(this);

        var infos = this.template.elementInfos;
        var ni = infos.length;
        for (var i = 0; i < ni; i++) 
        {
        	var info = infos[i];
        	var element = traverseSubElement(this, info.path);

        	if (info.bindingFunc !== null)
        	{
        		var binding = new Binding();
				binding.element = element;
				binding.func = info.bindingFunc;
				this.bindings.push(binding);
        	}

        	for (var b = 0; b < info.behaviors.length; b++) 
        	{
        		info.behaviors[b].attached(element);
        	};

        	// TODO: need double check this is custom element?
        	for (var p = 0; p < info.properties.length; p++) 
        	{
        		var prop = info.properties[p];
        		prop.property.set(element, prop.value);   		
        	};
        };

        //this.bind(null);
	};

	customElemProto.bind = function(data)
	{
		var oldData = this.dataContext;

		if (oldData === data)
			return;

		observeData(this, data);
		refreshBindings(this);

		this.dispatchEvent(new CustomEvent("bindingchanged", { detail: { previous: oldData, current: data } }));
	};

	// customElemProto.property = function(name, value)
	// {
	// 	if (arguments.length >= 2)
	// 	{
	// 		if (value === undefined)
	// 			throw ("cannot set property to undefined.");

	// 		if (this.properties === null)
	// 			this.properties = {};

	// 		this.properties[name] = value;
	// 	}
	// 	else
	// 	{
	// 		if (this.properties !== null)
	// 		{
	// 			value = this.properties[name];
	// 		}
	// 	}
	// 	return value;
	// };

	// function attachBehaviors(element)
	// {
	// 	var behaviors = element.template.behaviors;
	// 	var numBehaviors = behaviors.length;
	// 	for (var i = 0; i < numBehaviors; i++) 
	// 	{
	// 		behaviors[i].attached(element);
	// 	};
	// };

	// function initBindings(element)
	// {
	// 	var infos = element.template.bindingInfos;
	// 	var numBindings = infos.length;
	// 	for (var i = 0; i < numBindings; i++) 
	// 	{
	// 		var info = infos[i];
	// 		var binding = new Binding();
	// 		binding.element = traverseSubElement(element, info.path);
	// 		binding.func = info.func;
	// 		element.bindings.push(binding);
	// 	};

	// 	// bind to null initially;
	// 	element.bind(null);
	// };

	function refreshBindings(element)
	{
		var data = element.dataContext;
		var numBindings = element.bindings.length;
		for (var i = 0; i < numBindings; i++) 
		{
			element.bindings[i].apply(data);
		};

		// pass to behavior
		element.dispatchEvent(new CustomEvent("datachanged", { detail: { data: data } }));	

		// if no bindings and no behavior hanlded, set to content (must be string, must have container)
	}

	function observeData(element, data)
	{
		if (element.dataContext !== null && element.dataObserver !== null)
			Object.unobserve(element.dataContext, element.dataObserver);

		element.dataContext = data;
		element.dataObserver = null;

		if (data !== null && typeof data === 'object')
		{
			element.dataObserver = function()
			{
				refreshBindings(element);
			};

			Object.observe(data, element.dataObserver);
		}
	};

	function traverseSubElement(element, path)
	{
		// 0 path pointing to the element itself.
		if (path.length === 0)
			return element;

		var cursor = element.shadowRoot;
		var l = path.length;
		for (var i = 0; i < l; i++) 
		{
			cursor = cursor.children[path[i]];
		};
		return cursor;
	};

	var BindingInfo = Class(
	{
		constructor : function(path, func)
		{
			this.path = path.concat();
			this.func = func;
		}
	});

	var Binding = Class(
	{
		constructor : function()
		{
			this.element = null;
			this.func = null;
		},

		apply : function(data)
		{
			var value;

			// TODO: how to make this more visible??
			//try
			{
				value = this.func(data);
			}
			// catch (e)
			// {
			// 	value = null;
			// 	console.error("Failed to evaluate binding expression in element '" + this.element.outerHTML + "', error was: " + e.message);
			// }

			// If custom element
			if (isCustomElement(this.element))
			{
				this.element.bind(value);
			}
			// Otherwise set content to string
			else
			{
				var html = value !== null ? value.toString() : "";
				this.element.innerHTML = html;
			}
		}
	});

// .########..########.##.....##....###....##.....##.####..#######..########.
// .##.....##.##.......##.....##...##.##...##.....##..##..##.....##.##.....##
// .##.....##.##.......##.....##..##...##..##.....##..##..##.....##.##.....##
// .########..######...#########.##.....##.##.....##..##..##.....##.########.
// .##.....##.##.......##.....##.#########..##...##...##..##.....##.##...##..
// .##.....##.##.......##.....##.##.....##...##.##....##..##.....##.##....##.
// .########..########.##.....##.##.....##....###....####..#######..##.....##



	var Behavior = Class(
	{
		constructor : function Behavior(name, config)
		{
			this.name = name;
			this.listeners = [];

			var keys = Object.keys(config);
			for (var i = 0; i < keys.length; i++) 
			{
				var key = keys[i];
				var value = config[key];

				if (typeof value === 'function')
				{
					if (key.startsWith("on"))
					{
						this.listeners.push(new BehaviorListener(this, key.substr(2), value));
					}
					else
					{
						this[key] = value;
					}
				}
				else if (typeof value === 'object')
				{
					if (value.type === undefined || value.value === undefined)
						throw (`incorrect behavior property format: ${key} : ${JSON.stringify(value)}`);

					var readonly = (value.readonly === false ? false : true); // make non-readonly explicit.
					this[key] = new BehaviorProperty(defineProperty(key, value.type, readonly), value.value);
				}
				else
				{
					throw (`Unexpected behavior config: ${key} : ${JSON.stringify(value)}`);
				}
			};
		},

		attached : function(element)
		{
			// var l = this.properties.length;
			// for (var i = 0; i < l; i++) 
			// {
			// 	var prop = this.properties[i];
			// 	if (element.property(prop.key) !== undefined)
			// 		throw ("Behavior property conflict: " + prop.key);

			// 	element.property(prop.key, prop.value);
			// };

			var l = this.listeners.length;
			for (var i = 0; i < l; i++) 
			{
				var listener = this.listeners[i];
				element.addEventListener(listener.type, listener.handler);
			};
		}
	});

	var BehaviorListener = Class(
	{
		constructor : function BehaviorListener(behavior, type, handler)
		{
			this.type = type;
			this.handler = function(event)
			{
				handler.call(behavior, event.currentTarget, event);
			};
		}
	});

	var BehaviorProperty = Class(
	{
		constructor : function BehaviorProperty(prop, value)
		{
			if (!prop || value === undefined)
				throw ("missing property or value.");

			this.property = prop;
			this.value = this.property.coerce(value);
		},

		get : function(element)
		{
			return this.property.get(element, this.value);
		},

		set : function(element, value)
		{
			return this.property.set(element, value);
		}
	});

	function findBehavior(name)
	{
		return behaviors[name] || null;
	};


// .########..########...#######..########..########.########..########.##....##
// .##.....##.##.....##.##.....##.##.....##.##.......##.....##....##.....##..##.
// .##.....##.##.....##.##.....##.##.....##.##.......##.....##....##......####..
// .########..########..##.....##.########..######...########.....##.......##...
// .##........##...##...##.....##.##........##.......##...##......##.......##...
// .##........##....##..##.....##.##........##.......##....##.....##.......##...
// .##........##.....##..#######..##........########.##.....##....##.......##...

	var CustomProperty = Class(
	{
		constructor : function CustomProperty(name, readonly)
		{
			this.name = name;
			this.readonly = readonly;
		},

		get : function(element, defaultValue)
		{
			var value;

			if (isCustomElement(element))
			{
				if (this.readonly)
				{
					value = element.dataset[this.name];
					if (value !== undefined)
						return this.parse(value);

					value = element.template.element.dataset[this.name];
					if (value !== undefined)
					{
						return this.parse(value);
					}
				}
				else if (element.properties !== null) 
				{
					value = element.properties[this.name];
					if (value !== undefined)
						return value;
				}
			}
			else
			{
				value = element.dataset[this.name];
				if (value !== undefined)
					return this.parse(value);
			}

			return defaultValue;
		},

		set : function(element, value)
		{
			if (this.readonly)
				throw ("Cannot set readonly property: " + this.name);

			if (isCustomElement(element))
			{
				if (element.properties === null)
					element.properties = {};

				element.properties[this.name] = value;
			}
			else
			{
				element.dataset[this.name] = this.stringify(value);
			}
		}
	});

	var StringProperty = Class(CustomProperty,
	{
		constructor : function StringProperty(name, readonly)
		{
			StringProperty.$super.call(this, name, readonly);
		},

		parse : function(str)
		{
			return str;
		},

		stringify : function(value)
		{
			return value;
		},

		coerce : function(value)
		{
			return (value + "");
		}
	});

	var NumberProperty = Class(CustomProperty,
	{
		constructor : function NumberProperty(name, readonly)
		{
			NumberProperty.$super.call(this, name, readonly);
		},

		parse : function(str)
		{
			return (str * 1);
		},

		stringify : function(value)
		{
			return (value + "");
		},

		coerce : function(value)
		{
			return (value * 1);
		}
	});

	function defineProperty(name, type, readonly)
	{
		if (properties[name] !== undefined)
			throw (`property with name ${name} already defined.`);

		readonly = readonly ? true : false;

		var prop = null;
		if (type === String)
		{
			prop = new StringProperty(name, readonly);	
		}
		else if (type === Number)
		{
			prop = new NumberProperty(name, readonly);
		}
		else
		{
			throw (`unknown property type ${type}`);
		}
		properties[name] = prop;
		return prop;
	};

	function findProperty(name)
	{
		return properties[name] || null;
	};

})(this);




























var UI = new (function(global)
{
	var templatesData = {};
	var templates = {};
	var rootDOM = null;

	this.screen = null;

	this.Template = function(data)
	{
		if (data[0] === undefined)
			throw ("invalid template data!");

		templatesData[data[0]] = data;
	};

	this.init = function(root)
	{
		rootDOM = root;
		this.initTemplates();
	};

	this.showScreen = function(screen)
	{
		var template = templates[screen];
		if (!template)
			throw ("cannot find screen template: " + template);
		this.screen = template.instance();
		rootDOM.append(this.screen.$DOM);
		return this.screen;
	};

	this.initTemplates = function()
	{
		// TODO: may not need 2 steps anymore
		for (var name in templatesData)
		{
			templates[name] = new Template();
		};

		for (var name in templatesData)
		{
			compile(name);
		};
	};


	var cssKeys = 
	{
		display : 1, margin: 1, padding: 1, "flex-direction" : 1, flex : 1,
		position : 1, left: 1, right: 1, top: 1, bottom: 1, 
		width : 1, height : 1, "max-width" : 1, "max-height" : 1
	};

	this.Element = Class(
	{
		constructor : function()
		{
			this.DOM = this.createDOM();
			this.$DOM = $(this.DOM);
			this.DOM.$elem = this;
			this.container = null;
			this.data = null;
			this.bindings = null;
			this.classBindings = null;
		},

		createDOM : function()
		{
			return $("<div></div>")[0];
		},

		setup : function(props)
		{
			var cssProps;
			for (var name in props)
			{
				var handler = cssKeys[name];
				if (handler === undefined)
					continue;

				cssProps = cssProps || {};
				if (typeof handler === 'function')
					cssProps[name] = handler(props[name]);
				else
					cssProps[name] = props[name];
			}

			if (cssProps)
				this.css(cssProps);
		},

		init : function()
		{
			
		},

		setData : function(data)
		{
			this.data = data;
			this.refresh();
		},

		refresh : function()
		{
			if (this.classBindings !== null)
			{
				var l = this.classBindings.length;
				for (var i = 0; i < l; i++) 
				{
					var binding = this.classBindings[i];
					var sub = binding[0];
					var prevClasses = binding[1];
					if (prevClasses) sub.removeClass(prevClasses);

					prevClasses = "";
						
					if (this.data !== null)
					{
						for (var b = 2; b < binding.length; b++) 
						{
							var cls = binding[b](this.data);
							if (cls) prevClasses += cls + " ";
						};
					}

					if (prevClasses) sub.addClass(prevClasses);
					binding[1] = prevClasses;
				};
			}

			if (this.bindings !== null)
			{
				var l = this.bindings.length;
				if (this.data !== null)
				{
					for (var i = 0; i < l; i++) 
					{
						var binding = this.bindings[i];
						var data = binding[1](this.data);
						if (data === undefined)
							throw ("invalid data from binding!");
						binding[0].setData(data);
					};
				}
				else
				{
					for (var i = 0; i < l; i++) 
					{
						this.bindings[i][0].setData(null);
					};
				}
			}
		},

		addBinding : function(sub, binding)
		{
			if (!this.bindings)
				this.bindings = [];
			this.bindings.push([sub, binding]);
		},

		addClassBinding : function(sub, bindings)
		{
			if (!this.classBindings)
				this.classBindings = [];
			this.classBindings.push([sub, ""].concat(bindings));
		},

		addClass : function(className)
		{
			this.$DOM.addClass(className);
		},

		removeClass : function(className)
		{
			this.$DOM.removeClass(className);
		},

		attr : function(name, value)
		{
			this.$DOM.attr(name, value);
		},

		addChild : function(child)
		{
			var container = this.getInnerMostContainer();
			if (container === null)
				throw ("cannot append, no container inside the element");
			container.append(child);
		},

		getInnerMostContainer : function()
		{
			var elem = this;
			while (elem !== null)
			{
				if (elem.container === elem)
					return elem;
				elem = elem.container;
			}
			return null;
		},

		append : function(inner)
		{
			this.$DOM.append(inner.$DOM);
		},

		detach : function()
		{
			this.$DOM.detach();
		},

		setSub : function(name, sub)
		{
			if (this[name] !== undefined)
				throw ("cannot add sub named " + name + "such property already exist!");
			this[name] = sub;
		},

		css : function(prop, value)
		{
			return this.$DOM.css(prop, value);
		},

		html : function(str)
		{
			return this.$DOM.html(str);
		},

		position : function(x, y, z)
		{
			var style = { left : x, top : y };
			if (z) style["z-index"] = z;

			this.$DOM.css(style);
		},

		width : function(v)
		{
			return arguments.length ? this.$DOM.width(v) : this.$DOM.width();
		},

		height : function(v)
		{
			return arguments.length ? this.$DOM.height(v) : this.$DOM.height();
		},

		on : function(event, handler, obj)
		{
			var self = this;
			this.$DOM.on(event, function(e)
			{
				self.handleEvent(e, handler, obj);
			});
		},

		handleEvent : function(e, handler, obj)
		{
			e.targetElement = e.target.$elem;
			if (!e.targetElement)
				throw ("event triggered from unbound DOM!");

			handler.call(obj, e);
		}
	});

	var Template = Class(
	{
		$statics:
		{
			Node : Class(
			{
				constructor : function()
				{
					this.$ref = true;
					this.template = null;
					this.id = null;
					this.parent = null;
					this.index = -1;
					this.parentIdx = -1;
					this.numDescendants = 0;
					this.isContainer = false;
					this.binding = null;
					this.props = null;
				}
			})
		},

		constructor : function()
		{
			this.$ref = true;
			this.name = "";
			this.compiling = false;
			this.compiled = false;
			this.element = null;
			this.nodes = [];
			this.ids = {};
		},

		instance : function(props)
		{
			props = props || this.getProps() || {};
			var elem = new this.element();
			var container = this.nodes[0].isContainer ? elem : null;

			elem.addClass(this.name);
			elem.setup(props);

			var l = this.nodes.length;
			var subs = [elem];
			for (var i = 1; i < l; i++) 
			{
				var node = this.nodes[i];
				var sub = node.template.instance(node.props);
				subs.push(sub);
				
				if (node.parentIdx === 0)
					elem.append(sub);
				else
					subs[node.parentIdx].addChild(sub);

				if (node.id)
				{
					sub.attr("id", node.id);
					elem.setSub(node.id, sub);	
				}

				if (node.isContainer)
					container = sub;

				if (node.binding !== null)
					elem.addBinding(sub, node.binding);
			};

			for (var i = 0; i < l; i++) 
			{
				var node = this.nodes[i];
				var sub = subs[i];
				if (node.classStr)
					sub.addClass(node.classStr);

				if (node.classBindings)
					elem.addClassBinding(sub, node.classBindings);
			};

			elem.container = container;
			return elem;
		},

		getProps : function()
		{
			return this.nodes[0].props;
		},

		findDescendantBranch : function(ancestorIdx, descendantIdx)
		{
			var descendant = this.nodes[descendantIdx];
			var current = descendant;
			while (current.parentIdx !== ancestorIdx)
			{
				if (current.parentIdx === -1)
					return -1;

				current = this.nodes[current.parentIdx];
			}

			var last = ancestorIdx + this.nodes[ancestorIdx].numDescendants;
			var branch = 0;
			for (var childIdx = ancestorIdx + 1; childIdx <= last; childIdx += this.nodes[childIdx].numDescendants + 1) 
			{
				if (childIdx === current.index)
					return branch;

				branch++;
			};

			throw ("found direct child but not branch! something went wrong!");
		}
	});

	var OVERRIDE_NONE = 0;
	var OVERRIDE_EXTEND = 1;
	var OVERRIDE_REPLACE = 2;

	function compile(templateName)
	{
		var template = templates[templateName];
		var data = templatesData[templateName];

		if (!template || !data)
			throw ("cannot find template named " + templateName);

		if (template.compiled)
			return template;

		if (template.compiling)
			throw ("circular dependency detected!");

		template.name = templateName;
		template.compiling = true;

		var name = data[0];
		var templateProps = data.length >= 2 && !isArray(data[1]) ? data[1] : {};
		template.element = templateProps.$element || UI.Element;

		var base = templateProps.$base || null;
		var baseTemplate = null;
		//var root = new Node();
		if (base !== null)
		{
			base = parseHeader(base);
			baseTemplate = compile(base.template);	
			data.$base = 0;
			data.$override = base.override || OVERRIDE_EXTEND;
		}

		delete templateProps.$element;
		delete templateProps.$base;

		data.$parent = null;
		data.$sibling = null;

		var current = data;
		while (current)
		{
			var children;
			var header = current[0];
			var hasProps = current.length >= 2 && !isArray(current[1]);
			var props = hasProps ? current[1] : null;
			current.splice(0, hasProps ? 2 : 1);

			if (hasProps && current === data)
			{
				if (props.$binding)
				{
					delete props.$binding;
					console.error("template root node cannot have binding!");
				}
			}

			if (current.$base !== undefined)
			{
				children = compileOverride(current, template, baseTemplate);
			}
			else
			{
				current.$node = new Template.Node();
				children = current;

				if (current !== data)
				{
					headerInfo = parseHeader(header);
					if (headerInfo.override !== OVERRIDE_NONE)
						throw ("cannot override a node that has no base!");
					current.$node.template = compile(headerInfo.template);
					current.$node.id = headerInfo.id;
				}
			}

			var node = current.$node;
			node.parent = current.$parent ? current.$parent.$node : null;
			node.isContainer = hasProps && props.hasOwnProperty("$container") ? props.$container : node.isContainer;
			node.binding = hasProps && props.hasOwnProperty("$binding") ? compileBinding(props.$binding) : node.binding;
			compileClass(node, props);
			template.nodes.push(node);

			// TODO: optimization: if has props but props is empty, ignore
			if (hasProps)
			{
				delete props.$container;
				delete props.$binding;
				delete props.$class;
				Cloner.replaceReferences(props, templates);
				baseProps = node.props || (node.template ? node.template.getProps() : null);
				node.props = baseProps ? Cloner.extendBatch(true, {}, baseProps, props) : props;				
			}
			
			var cl = children ? children.length : 0;
			for (var i = 0; i < cl; i++) 
			{
				children[i].$parent = current;
				children[i].$sibling = (i < cl - 1) ? children[i+1] : null;
			};

			if (cl > 0)
			{
				current = children[0];
			}
			else
			{
				while (current !== null)
				{
					if (current.$sibling)
					{
						current = current.$sibling;
						break;
					}
					current = current.$parent;
				}
			}
		};

		var nl = template.nodes.length;
		for (var i = 0; i < nl; i++) 
		{
			template.nodes[i].index = i;
		};

		depth = [0];
		for (var i = 1; i < nl; i++) 
		{
			var node = template.nodes[i];
			var parentIdx = node.parent.index;
			node.parentIdx = parentIdx;

			if (node.id !== null)
				template.ids[node.id] = i;

			for (var d = depth.length - 1; d >= 0; d--)
			{
				if (depth[d] === parentIdx)
					break;

				var ancestorIdx = depth.pop();
				template.nodes[ancestorIdx].numDescendants = (i - 1) - ancestorIdx;
			};

			// TODO: assert
			if (depth.length === 0)
				throw ("root should always be in depth!");

			depth.push(i);
		};

		for (var d = depth.length - 1; d >= 0; d--)
		{
			var ancestorIdx = depth[d];
			template.nodes[ancestorIdx].numDescendants = (nl - 1) - ancestorIdx;
		}

		template.compiling = false;
		template.compiled = true;
		return template;
	};

	function parseHeader(header)
	{
		var info = {};

		header = header.trim();

		switch (header[0])
		{
			case '+': info.override = OVERRIDE_EXTEND; break;
			case '*': info.override = OVERRIDE_REPLACE; break;
			default: info.override = OVERRIDE_NONE; break;
		}

		if (info.override !== OVERRIDE_NONE)
			header = header.substr(1);

		var tokens = header.split(/\s*#\s*/);
		info.template = tokens[0];
		info.id = tokens.length >= 2 ? tokens[1] : null; 
		return info
	};

	function compileOverride(current, template, baseTemplate)
	{
		var baseIdx = current.$base;
		var baseNode = baseTemplate.nodes[baseIdx];
		var node = current.$node = Cloner.clone(baseNode);
		var children = null;

		if (current.$override === OVERRIDE_EXTEND)
		{
			var nd = baseNode.numDescendants;

			if (current.length === 0)
			{
				var baseStartIdx = baseIdx + 1;
				var clonedStartIdx = template.nodes.length;
				for (var i = 0; i < nd; i++) 
				{
					var baseDescendant = baseTemplate.nodes[baseStartIdx+i];
					var descendant = Cloner.clone(baseDescendant);
					var parentIdx = baseDescendant.parentIdx - baseStartIdx + clonedStartIdx;

					// TODO: this should be assert
					if (parentIdx < clonedStartIdx - 1)
						throw ("the descendant has parent beyond the current node, something went wrong!");

					descendant.parent = template.nodes[parentIdx];
				};
			}
			else
			{
				children = [];
				var childIdx = baseIdx + 1;
				while (childIdx <= baseIdx + nd)
				{
					var autoChild = ["$auto"];
					autoChild.$base = childIdx;
					autoChild.$override = OVERRIDE_EXTEND;
					autoChild.$auto = true;
					children.push(autoChild);
					childIdx += baseTemplate.nodes[childIdx].numDescendants + 1;
				};

				for (var i = 0; i < current.length; i++) 
				{
					var child = current[i];
					var childInfo = parseHeader(child[0]);

					if (childInfo.override === OVERRIDE_EXTEND || childInfo.override === OVERRIDE_REPLACE)
					{
						var childBaseIdx = baseTemplate.ids[childInfo.id];
						if (childBaseIdx === undefined)
							throw ("cannot find base to override, with id " + childInfo.id);

						var branchIdx = baseTemplate.findDescendantBranch(childBaseIdx);
						if (branchIdx < 0)
							throw ("override node not a descendant of current! rearrange! " + childInfo.id);

						var branch = children[branchIdx];
						if (!branch.$auto)
							throw ("attempting to insert override in another override! must rearrange " + childInfo.id);

						child.$base = childBaseIdx;
						child.$override = childInfo.override;

						if (childBaseIdx === branch.$base)
						{
							if (branch.length >= 2)
								throw ("inserted override into branch, but trying override it as a whole, rearrange! " + childInfo.id);
							children[branchIdx] = child;
						}
						else
						{
							branch.push(child);
						}
					}
					else if (childInfo.override === OVERRIDE_NONE)
					{
						children.push(child);
					}
					else
					{
						throw ("unhandled override type: " + childInfo.override);
					}
				};
			}
		}
		else if (current.$override === OVERRIDE_REPLACE)
		{
			children = current;
		}
		else
		{
			throw ("unhandled override type: " + current.$override);
		}

		return children;
	};

	function compileBinding(binding) 
	{
		binding.trim();

		if (binding.length === 0)
			return null;

		if (binding.search(/[.()\[\]]/) === -1)
		{
			return function(data) { return data[binding]; }
		}
		else
		{
			return new Function("data", "return data." + binding);
		}
	};

	function compileClass(node, props)
	{
		if (!props || !props.$class)
			return;

		var classes = props.$class.split(/\s+/);
		var classStr = "";
		var classBindings = [];
		for (var i = 0; i < classes.length; i++) 
		{
			var cls = classes[i];
			if (!cls) continue;

			if (cls[0] === '$')
			{
				classBindings.push(compileBinding(cls.substr(1)));
			}
			else
			{
				if (classStr) classStr += " ";
				classStr += cls;
			}
		};

		if (classStr)
		{
			node.classStr = node.classStr ? (node.classStr + " " + classStr) : classStr;
		}

		if (classBindings.length > 0)
		{
			if (node.classBindings)
				node.classBindings = node.classBindings.concat(classBindings);
			else
				node.classBindings = classBindings;
		}
	}
	
})(this);