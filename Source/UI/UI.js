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
			return $("<div />")[0];
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