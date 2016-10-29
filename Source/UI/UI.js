/* global UI: true */
/* exported UI */

var UI = new (function(global)
{
	'use strict';

	var numLoadingHtml = 0;
	var readyCallback = null;
	var templates = {};
	var behaviors = {};
	var directives = {};
	var properties = {};
	var customEvents = {};

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
				}

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

	this.Directive = function(name, config, cls)
	{
		let directive = new DirectiveDefinition(name, config, cls);
		if (directives[name] || name === Keyword.end)
			throw (`The directive with name ${name} is already registered.`);

		directives[name] = directive;
		return directive;
	};

	this.calcElementOffset = calcElementOffset;

	this.findTemplate = function(name)
	{
		return templates[name] || null;
	};


// ████████╗███████╗███╗   ███╗██████╗ ██╗      █████╗ ████████╗███████╗
// ╚══██╔══╝██╔════╝████╗ ████║██╔══██╗██║     ██╔══██╗╚══██╔══╝██╔════╝
//    ██║   █████╗  ██╔████╔██║██████╔╝██║     ███████║   ██║   █████╗  
//    ██║   ██╔══╝  ██║╚██╔╝██║██╔═══╝ ██║     ██╔══██║   ██║   ██╔══╝  
//    ██║   ███████╗██║ ╚═╝ ██║██║     ███████╗██║  ██║   ██║   ███████╗
//    ╚═╝   ╚══════╝╚═╝     ╚═╝╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝   ╚══════╝



	class Template
	{
		constructor(templateElement)
		{
			this.name = templateElement.id.toLowerCase(); // tag is case-insensitive, but keep lower case for lookup.
			this.element = templateElement;
			this.proto = Object.create(CustomElement.prototype);			
			//this.elementInfos = [];
			this.dependencies = {};
			//this.dataClass = templateElement.dataset["class"] || "";
			//this.bindingInfos = [];
			//this.behaviors = [];
			
			var template = this;
			this.proto.createdCallback = function()
			{
				CustomElement.apply(this);
				this.setup(template); // TODO: should we do this in attachedCallback
			};

			this.proto.attachedCallback = function()
			{
				// if (this.customEvents.elementattached)
				// 	this.customEvents.elementattached({currentTarget:this});
				//this.dispatchEvent(new CustomEvent("elementattached"));
				this.dispatchCustomEvent(new ElementAttached());
			};

			this.proto.detachedCallback = function()
			{
				// TODO: data clean up, and detach events
				//this.bind(null);
			};


			this.componentDef = parseComponent(templateElement.content, this.dependencies);
			parseComponentElement(this.componentDef, templateElement, [], false);

			
			//this.collectElementInfo(this.element, []);
			//this.collectChildrenInfos(this.element.content.children, []);
		}

/*
		collectElementInfo : function(element, path)
		{
			var i;
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
				for (i = 0; i < behaviorNames.length; i++) 
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
				}
			}

			// TODO: do not allow non-custom element to have custom attributes
			// Although here you can't check element.template to know if it's
			// custom or not, must check if it has - in the tag
			var attrs = element.attributes;
			for (i = 0; i < attrs.length; i++) 
			{
				var attr = attrs[i];
				if (attr.name.length > 0 && attr.name[0] === '$')
				{					
					if (attr.name.startsWith("$on-"))
					{
						var eventBinding = new EventBindingInfo();
						eventBinding.type = attr.name.substr(4);
						eventBinding.listener = this.compileEventListener(attr.value);
						info.eventBindings.push(eventBinding);
					}
					else
					{
						var key = attrToPropName(attr.name.substr(1));
						var prop = findProperty(key);
						if (prop)
						{
							var propFunc = this.compileBinding(attr.value);
							if (propFunc !== null)
							{
								var propBinding = new ElementPropertyBinding();
								propBinding.property = prop;
								propBinding.func = propFunc;
								info.propBindings.push(propBinding);
							}
							else if (prop.readonly || !isCustomElement)
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
					}

					element.removeAttribute(attr.name);
					i--;
				}
			}

			// if (info.properties.length > 0 && !isCustomElement)
			// {
			// 	console.error("Cannot apply custom properties to non-custom element: " + element.tagName);
			// 	info.properties.length = 0; 
			// }

			if (!info.isEmpty())
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
			}
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
		},

		compileEventListener : function(expression)
		{
			if (expression.startsWith("->"))
			{
				return this.buildRelayListener(expression.substr(2));
			}
			else
			{
				var body = "var data = this.dataContext;" +
						   "if (data !== null) { data." + expression + "; }";

				return new Function("e", body);
			}
		},

		buildRelayListener: function(nextEventType)
		{
			return function(e)
			{
				e.stopImmediatePropagation();
				e.currentTarget.dispatchEvent(
					new CustomEvent(nextEventType, { bubbles: true, detail: { data: this.dataContext } }));
			};
		}*/
	}	

/*
	var ElementInfo = Class(
	{
		constructor : function ElementInfo()
		{
			this.path = null;
			this.bindingFunc = null;
			this.behaviors = [];
			this.properties = [];
			this.propBindings = []; 
			this.eventBindings = [];
		},

		isEmpty: function()
		{
			return (this.bindingFunc === null && 
				this.behaviors.length === 0 && this.properties.length === 0 && 
				this.propBindings.length === 0 && this.eventBindings.length === 0);
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

	var ElementPropertyBinding = Class(
	{
		constructor : function ElementPropertyBinding()
		{
			this.property = null;
			this.func = null;
		}
	});
*/
	var pendingTemplates = {};

	function registerTemplate(templateElement)
	{
		var template = new Template(templateElement);
		var dependencies = Object.keys(template.dependencies);			
		if (dependencies.length > 0)
		{
			for (var i = 0; i < dependencies.length; i++) 
			{
				var dependency = dependencies[i];
				if (pendingTemplates[dependency] === undefined)
					pendingTemplates[dependency] = [];

				pendingTemplates[dependency].push(template);
			}
		}
		else
		{
			finishRegisteringTemplate(template);
		}
		
		return template;
	}

	function finishRegisteringTemplate(template)
	{
		var name = template.name;

		if (template[name])
		{
			console.error("Duplicated template registeration: " + name);
			return;
		}

		document.registerElement(name, { prototype: template.proto });
		templates[name] = template;

		var pendings = pendingTemplates[name];
		if (pendings !== undefined)
		{
			for (var i = 0; i < pendings.length; i++) 
			{
				let pending = pendings[i];

				if (!pending.dependencies[name])
					throw ("Pending template no longer dependant on the dependency?");

				delete pending.dependencies[name];

				if (Object.keys(pending.dependencies).length === 0)
				{
					// optional: pending.dependencies = null;
					finishRegisteringTemplate(pending);
				}					
			}

			delete pendingTemplates[name];
		}
	}

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
	}

	function isCustomElement(element)
	{
		return !!element.template; //&& true;
	}


//  ██████╗ ██████╗ ███╗   ███╗██████╗  ██████╗ ███╗   ██╗███████╗███╗   ██╗████████╗
// ██╔════╝██╔═══██╗████╗ ████║██╔══██╗██╔═══██╗████╗  ██║██╔════╝████╗  ██║╚══██╔══╝
// ██║     ██║   ██║██╔████╔██║██████╔╝██║   ██║██╔██╗ ██║█████╗  ██╔██╗ ██║   ██║   
// ██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██║   ██║██║╚██╗██║██╔══╝  ██║╚██╗██║   ██║   
// ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ╚██████╔╝██║ ╚████║███████╗██║ ╚████║   ██║   
//  ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝      ╚═════╝ ╚═╝  ╚═══╝╚══════╝╚═╝  ╚═══╝   ╚═╝   

// component definion
//	- compile and parse the given content (DocumentFragment)
//	- compile the binding list (element -> binding func)
//	- compile the directives (each directive will be assigned a placeholder element, 
//	  and the content will be put into a doc frag and compiled as a new component)
//	- compile the events
//	- compile scripts
//
// component (instance)
//	- component will be instanced upon a set of root nodes (and their descendants) attached using the doc frag
//	- component will then assign certain placeholder nodes to the directives, so the total root nodes may grow.
//	- the initial data for the component will be null, so all bindings and directives will be operating on a default value.
//	- when data is set, all bindings are refreshed, and all directives update.
//
// data binding
//	- content binding (always string? to assigned to text content? will compare and ignore if equal to the old)
//	- attribute binding (also always string, pretty much the same as content binding)
//	- property binding (only bindable to a custom element, a directive and its component,
//	  does not matter if the value changes or not, always force update the corresponding element/directive/component)
//	  UNLESS maybe when it's premitive value, then we can ignore?
//	- some directives will assign the entire parent's property set to the child components, in that case, 
//	  just assigning the set object instead of copying each property is probably fine.

	class Component
	{
		constructor(definition)
		{
			this.definition = definition;
			this.host = null;
			this.firstNode = null;
			this.lastNode = null;
			this.data = null;
			this.bindings = [];
			this.directives = [];
			//this.eventReceipt = null;
		}

		isAttached()
		{
			return this.firstNode !== null;
		}

		attach(parentNode, beforeThisNode, host)
		{
			if (this.isAttached())
				throw ("Trying to attach the component again when it's already attached.");

			let clonedContent = document.importNode(this.definition.content, true);
			if (!clonedContent || !clonedContent.firstChild)
				throw ("Invalid component content");

			this.firstNode = clonedContent.firstChild;
			this.lastNode = clonedContent.lastChild;

			if (!parentNode)
				throw ("Component must be attached to a parent node.");

			if (beforeThisNode && beforeThisNode.parentNode !== parentNode)
				throw ("The 'before this' node must be the child of the parent node.");

			parentNode.insertBefore(clonedContent, beforeThisNode);

			// hook up the following:
			// custom element property bindings
			// attribute binding
			// text bindings
			// event bindings
			// directives

			this.bindings.length = 0;
			let bindingsDef = this.definition.bindings;
			for (let i = 0; i < bindingsDef.length; i++) 
			{
				let bindingDef = bindingsDef[i];
				let node = traverseNodePath(this.firstNode, host, bindingDef.path);
				bindingDef.init(node, this);
				this.bindings.push(new BindingInstance(node, bindingDef));
			}

			this.directives.length = 0;
			let directiveDefs = this.definition.directives;
			for (let i = 0; i < directiveDefs.length; i++) 
			{
				let directiveDef = directiveDefs[i];
				let node = traverseNodePath(this.firstNode, host, directiveDef.path);
				this.directives.push(new DirectiveInstance(node, node.nextSibling, directiveDef.directive));
			}

			this.data = {}; // TODO: user defined data class (with helper functions etc.)
			// TODO: if there's already data property set, we need make sure the refresh event is bound
		}

		detach()
		{
			if (!this.isAttached())
				throw ("Trying to detach the component when it's not yet attached.");

			let parent = this.firstNode.parentNode;
			let removingNode = this.firstNode;
			while (removingNode && removingNode !== this.lastNode)
			{
				let next = removingNode.nextSibling;
				parent.removeChild(removingNode);
				removingNode = next;
			}
			parent.removeChild(this.lastNode);

			this.bindings.length = 0;
			this.directives.length = 0;

			// TODO: clear and unbind data

			this.data = null;			
		}

		replace(replacedNode)
		{
			let parentNode = replacedNode.parentNode;
			if (!parentNode)
				throw ("To replace a node, the replaced node must have a parent.");

			this.attach(parentNode, replacedNode);
		}

		// setData(data)
		// {
		// 	for (let key in data)
		// 	{
		// 		this.setDataProp(key, data[key]);
		// 	}

		// 	this.refresh();
		// }

		transferData(data)
		{
			this.data = data;
		}

		setDataProp(key, value)
		{
			if (!this.data)
				throw ("Cannot set data property when not attached.");

			// TODO: hook up with the object (so the object can trigger component referesh)

			this.data[key] = value;
		}

		refresh()
		{
			let data = this.data;

			// apply each data binding
			let bindings = this.bindings;
			for (let i = 0; i < bindings.length; i++)
			{
				let bindingInst = bindings[i];
				bindingInst.binding.apply(bindingInst.node, data);
			}

			// refresh all directives
			let directives = this.directives;
			for (let i = 0; i < directives.length; i++) 
			{
				let directiveInst = directives[i];
				directiveInst.directive.apply(directiveInst, data);
			}
		}
	}

	function traverseNodePath(first, host, path)
	{
		if (path.length === 0)
		{
			if (!host)
				throw ("Empty path without host.");
			return host;
		}

		let firstOffset = path[0];
		let node = first;
		for (let i = 0; i < firstOffset; i++) 
		{
			node = node.nextSibling;
		}

		//let node = parent;
		for (let i = 1; i < path.length; i++) 
		{
			node = node.childNodes[path[i]];
		}

		if (!node)
			throw ("Invalid path!");

		return node;
	}

	class BindingInstance
	{
		constructor(node, binding)
		{
			this.node = node;
			this.binding = binding;
		}
	}

	class EventBindingInfo
	{
		constructor()
		{
			this.type = "";
			this.listener = null;
		}
	}

	class AttributeBindingInfo
	{
		constructor()
		{
			this.name = "";
			this.func = null;
		}
	}

	class ElementBinding
	{
		constructor(path)
		{
			this.path = path.concat();
			this.eventBindings = [];
			this.attributeBindings = [];
			this.propertyBindings = [];
		}

		init(node, component)
		{
			for (let i = 0; i < this.eventBindings.length; i++) 
			{
				let eventBinding = this.eventBindings[i];

				// TODO: if we reuse node, then we must remove the listener first otherwise the event will be bound twice.
				// TODO: bind() is slow, avoid using or at least replace it with a simpler version.
				node.addEventListener(eventBinding.type, eventBinding.listener.bind(component));
			}
		}

		apply(node, data)
		{
			for (let i = 0; i < this.attributeBindings.length; i++) 
			{
				let attrBinding = this.attributeBindings[i];
				// TODO: compare value before set? is there any perf implication when setting the same value?
				node.setAttribute(attrBinding.name, attrBinding.func.call(node, data));
			}

			if (this.propertyBindings.length > 0)
			{
				// NOTE: the node must be a custom element that has a "component" property.
				let component = node.component;
				for (let i = 0; i < this.propertyBindings.length; i++) 
				{
					let propBinding = this.propertyBindings[i];
					// TODO: compare value before set? is there any perf implication when setting the same value?
					component.setDataProp(propBinding.name, propBinding.func.call(node, data));
				}
				component.refresh();
			}
		}
	}

	class TextBinding
	{
		constructor(path, bindingFunc)
		{
			this.path = path.concat();
			this.func = bindingFunc;
		}

		init(node, component)
		{

		}

		apply(node, data)
		{
			// TODO: compare value before set? is there any perf implication when setting the same value?
			node.data = this.func.call(node, data);
		}
	}

	class DirectiveBinding
	{
		constructor(path, directive)
		{
			this.path = path.concat();
			this.directive = directive;
		}
	}

	class DirectiveInstance
	{
		constructor(beginNode, endNode, directive)
		{
			this.begin = beginNode;
			this.end = endNode;
			this.directive = directive;
			this.components = [];
		}

		populate(def, num)
		{
			num = num || 0;			

			if (!def && num > 0)
			{
				console.error("Cannot populate components from directive: the component definition is invalid.");
				return;
			}

			for (let i = this.components.length - 1; i >= 0; i--) 
			{				
				if (this.components[i].definition !== def)
				{
					this.components[i].detach();
					this.components.splice(i, 1);
				}
			}

			let oldNum = this.components.length;			
			if (oldNum < num)
			{
				let parent = this.end.parentNode;
				for (let i = oldNum; i < num; i++) 
				{
					let newComp = new Component(def);
					newComp.attach(parent, this.end, null);
					this.components.push(newComp);
				}
			}
			else
			{
				for (let i = oldNum - 1; i >= num; i--)
				{
					this.components[i].detach();
				}
				this.components.length = num;
			}
		}
	}

	// Should we rename this to Prefab
	class ComponentDefinition 
	{
		constructor(content)
		{
			this.content = content;
			this.bindings = [];
			this.directives = [];
		}
	}

	function parseComponent(content, dependencies)
	{
		if (!content)
			throw ("Invalid content.");

		let componentDef = new ComponentDefinition(content);

		// TODO: style not allowed for non-shadow-rooted component
		// TODO: handle and remove <script>			

		// TODO: trim head (skip style node) and tail white spaced text node (may or may not need)
		// maybe call normalize();

		let path = [];
		parseComponentNodes(componentDef, content, path, dependencies);
		return componentDef;
	}

	function parseComponentNodes(componentDef, parent, path, dependencies)
	{
		path.push(0);
		let node = parent.firstChild;
		//let parent = firstNode.parentNode;
		while (node !== null)
		{
			// Element
			if (node.nodeType === Node.ELEMENT_NODE)
			{
				let tagName = node.tagName.toLowerCase();
				let isCustomElement = (tagName.includes('-'));
				if (isCustomElement && !templates[tagName])
				{
					dependencies[tagName] = true;
				}

				// skip <style> and <script> 
				if (tagName !== "style" && tagName !== "script")
				{
					parseComponentElement(componentDef, node, path, isCustomElement);
					
					parseComponentNodes(componentDef, node, path, dependencies);
					//componentDef.processNodes(node, path);	
				}
			}
			// Text node
			else if (node.nodeType === Node.TEXT_NODE)
			{
				let originalText = node.data;

				// TODO: maybe we should tokenize and parse
				// - parse plain text and text binding
				// - until we reach a dirctive prefix #
				// - then will handle directive parsing for the remainder

				// search for directives # (not \#)
				// TODO: should be regex
				let reader = CreateTextLexer(originalText);
				let result = parseTextContent(reader);


				if (result)
				{
					let [modifiedText, bindingFunc] = result;
					
					node.data = modifiedText;
					
					if (bindingFunc)
					{
						// TODO: hook up text binding
						let textBinding = new TextBinding(path, bindingFunc);
						componentDef.bindings.push(textBinding);
					}


					// let directiveNode = null;
					// let directiveStart = text.indexOf("##");
					// if (directiveStart >= 0)
					// {
					// 	// cut off the parsed directives
					// 	directiveNode = node.splitText(directiveStart);
					// 	// should we delete this node if nothing left? (empty or all whitespaces) DONT DO THIS FOR NOW
					// }

					// to process the remaning text content:
					// - find any data binding content enclosed in {} (note not \{})
					// - process data binding the concatenate those with the plain text content
					// - for plain text content replace all \# with # and \{ with {

					// TODO: replace \## with ##

					// content bindings {{}}

					let nextToken = reader.peek();
					if (nextToken && nextToken.type === TokenType.directive)
					{
						let directiveNode = document.createTextNode(originalText.substr(nextToken.start));
						parent.insertBefore(directiveNode, node.nextSibling);

						let directive = parseDirective(directiveNode, dependencies);
						if (directive)
						{
							path[path.length-1]++;
							let directiveBinding = new DirectiveBinding(path, directive);
							componentDef.directives.push(directiveBinding);

							// TODO: add to directive list
							//parent.replaceChild(createPlaceholder(), directiveNode);
							directiveNode.data = ""; // hidden directive begin node.

							let directiveEndNode = directiveNode.nextSibling;
							if (!directiveEndNode)
								throw ("Found directive without end node.");

							directiveEndNode.data = ""; // hidden directive end node.

							// if directive parsed, a <placeholder> node will be added, skip that one.
							node = directiveEndNode; // skip these two already processed nodes.
							path[path.length-1]++;
						}
						else
						{
							console.error(`Failed to parse directive:\n${directiveNode.data}.\nOuter HTML:\n${parent.outerHTML || "[ROOT]"}`);
							// Failed to parse the given directive, simply remove this node, but expect for more errors to follow.
							parent.removeChild(directiveNode);
							// should we find til the next ##end, then delete all.
						}
					}
				}

				
			}
			// Other types not handled.

			// tentatively
			node = node.nextSibling;
			path[path.length-1]++;
		}

		path.pop();
	}

	function parseComponentElement(componentDef, node, path, isCustomElement)
	{
		if (!node.hasAttributes())
			return;

		let elementBinding = new ElementBinding(path);
		let attrs = node.attributes;
		let attrsToRemove = [];
		for (let i = 0; i < attrs.length; i++) 
		{
			let attr = attrs[i];
			let attrName = attr.name;
			if (attrName.startsWith("$on-"))
			{
				// event bindings
				attrsToRemove.push(attrName);
				let eventBinding = new EventBindingInfo();
				eventBinding.type = attrName.substr(4);
				eventBinding.listener = parseEventListener(attr.value);

				// TODO: add to element binding
				if (eventBinding.listener)
				{
					elementBinding.eventBindings.push(eventBinding);
				}

			}
			else if (attrName.startsWith('$'))
			{
				// property bindings （must be custom element)
				attrsToRemove.push(attrName);
				let propBinding = new AttributeBindingInfo();
				propBinding.name = attrName.substr(1);
				propBinding.func = parseDataBindingToFunc(attr.value, null);

				if (propBinding.func)
				{
					if (isCustomElement)
					{
						// TODO: add to element binding
						elementBinding.propertyBindings.push(propBinding);
				
					}
					else
					{
						console.error(`Property binding ${attrName}=${attr.value} found on node ${node.tagName}, which is not a custom element and does not support property binding. full content: ${node.outerHTML}`);
					}
				}
			}
			else if (attrName.toLowerCase() !== "id")
			{
				// attribute bindings (including id, class? id may be bit dangerous, but class is ok, not sure)
				let lexer = CreateTextLexer(attr.value);
				let result = parseTextContent(lexer);
				if (result)
				{
					if (!lexer.ended())
					{
						console.error(`Attribute ${attrName} has text remainer after parsing, which will be ignored: ${attr.value.substr(lexer.peek().start)}, full content: ${node.outerHTML}`);
					}

					let [modifiedAttrValue, bindingFunc] = result;
					if (bindingFunc)
					{
						attrsToRemove.push(attrName);
						// TODO: add to element binding
						let attrBinding = new AttributeBindingInfo();
						attrBinding.name = attrName;
						attrBinding.func = bindingFunc;
						elementBinding.attributeBindings.push(attrBinding);
					}
					else
					{
						attr.value = modifiedAttrValue;
					}
				}
			}
		}

		if (elementBinding.eventBindings.length > 0 ||
			elementBinding.propertyBindings.length > 0 ||
			elementBinding.attributeBindings.length > 0)
		{
			componentDef.bindings.push(elementBinding);
		}

		for (let i = 0; i < attrsToRemove.length; i++) 
		{
			node.removeAttribute(attrsToRemove[i]);
		}
	}

	var TokenType =
	{
		invalid 				: -1,
		directive				: 0,
		openBinding				: 1,
		closeBinding 			: 2,
		escapedSymbol			: 3,
		identifier				: 6,
		string 					: 7,
		openParen				: 8,
		closeParen				: 9,
		memberOp				: 10,
		number 					: 11,
		operator				: 12,		
	};

	var Keyword =
	{
		end 					: "end",
		escape 					: "\\",
	};

	class Token
	{
		constructor()
		{
			this.type = TokenType.invalid;
			this.str = "";
			//this.index = -1;
			this.start = -1;
			this.length = 0;
		}
	}

	class Lexer
	{
		constructor(str, regex, tokenMap)
		{
			this.str = str;
			this.regex = new RegExp(regex, "g");
			this.tokenMap = tokenMap;
			this.token = null;
			this.cursor = 0;
			this.next();
		}

		peek()
		{
			return this.token;
		}

		next()
		{
			let nextToken = this.token;

			let result = this.regex.exec(this.str);
			if (result)
			{
				let newToken = this.token = new Token();
				newToken.str = result[0];
				newToken.start = this.cursor = result.index;
				newToken.length = newToken.str.length;

				for (let i = 0; i < this.tokenMap.length; i++) 
				{
					if (result[i+1])
					{
						newToken.type = this.tokenMap[i];
						break;
					}
				}
			}
			else
			{
				this.cursor = this.regex.lastIndex = this.str.length;
				this.token = null;
			}

			return nextToken;
		}

		find(type)
		{
			while (this.token !== null)
			{
				let token = this.next();
				if (token.type === type)
					return token;
			}

			return null;
		}

		test(type)
		{
			if (this.token && this.token.type === type)
				return this.next();
			else
				return null;
		}

		ended()
		{
			return (this.token === null);
		}
	}


	//				."string"               .\\#{      .#directive           .{  .}       
	var textRegex = /("(?:[^"\\\n\r]|\\.)*")|(\\[\\#{])|(\s*#\s*[a-zA-Z]+\s*)|({)|(})/g;
	var textTokens = [TokenType.string, TokenType.escapedSymbol, TokenType.directive, 
		TokenType.openBinding, TokenType.closeBinding];

	function CreateTextLexer(str)
	{
		return new Lexer(str, textRegex, textTokens);
	}

	//					 ."string"			     .#directive 		   .identifier	   .(   .)   .number 				 .operators
	var directiveRegex = /("(?:[^"\\\n\r]|\\.)*")|(\s*#\s*[a-zA-Z]+\s*)|([$_a-zA-Z]\w*)|(\()|(\))|(-?(?:\d+\.?\d*|\.\d+))|([@#$%^&|+\-*/<>=\\[\]{}"':;,.?!])/g;
	var directiveTokens = [TokenType.string, TokenType.directive, TokenType.identifier,
		TokenType.openParen, TokenType.closeParen, TokenType.number, TokenType.operator];

	function CreateDirectiveLexer(str)
	{
		return new Lexer(str, directiveRegex, directiveTokens);
	}

	//				   ."string"	 		   .identifier		  .number 				  .mem .operator
	var bindingRegex = /("(?:[^"\\\n\r]|\\.)*")|(\$?[$_a-zA-Z]\w*)|(-?(?:\d+\.?\d*|\.\d+))|(\.)|([@#$%^&|+\-*/<>=\\[\]{}()"':;,?!])/g;
	var bindingTokens = [TokenType.string, TokenType.identifier, TokenType.number, TokenType.memberOp, TokenType.operator];

	function CreateDataBindingLexer(str)
	{
		return new Lexer(str, bindingRegex, bindingTokens);
	}

	var exprSyntaxRegex = /([a-zA-Z]+)|([@#$%^&|+\-*/<>=\\[\]{}()"':;,.?!])/g;
	var exprSyntaxTokens = [TokenType.identifier, TokenType.operator];

	function CreateDirectiveExpressionSyntaxLexer(str)
	{
		return new Lexer(str, exprSyntaxRegex, exprSyntaxTokens);
	}

	function parseTextContent(reader)
	{
		let plainTextStart = 0;
		let plainText = "";
		let compiledScript = "";

		while (true)
		{
			let token = reader.peek();
			if (!token)
			{
				break;
			}

			plainText += reader.str.substring(plainTextStart, token.start);
			plainTextStart = token.start + token.length;

			if (token.type === TokenType.directive)
			{
				// consume all remaining text (including plain text)
				plainTextStart = reader.str.length;

				// end parsing
				break;
			}
			else if (token.type === TokenType.openBinding)
			{
				// put current plain text in a text binding list (maybe can just be a string of JS text)
				if (compiledScript)
				{
					if (plainText)
						compiledScript += " + " + JSON.stringify(plainText);
				}
				else
				{
					compiledScript = JSON.stringify(plainText);
				}

				plainText = "";

				// Unmatching {}, fail
				let closeToken = reader.find(TokenType.closeBinding);
				if (!closeToken)
				{
					console.error(`Unmatched open brace: ${reader.str.substr(token.start)} \nwhile parsing text content:\n ${reader.str}`);
					return null;
				}

				let dataBindingStr = reader.str.substring(token.start + token.length, closeToken.start);
				plainTextStart = closeToken.start + closeToken.length;

				// parse this data binding, then add to the binding list too
				// TODO: we could also consider using the template literal
				// slice() strips out {}
				let dataBindingExpr = parseDataBindingToScript(dataBindingStr);
				if (!dataBindingExpr) // error.
					return null;

				compiledScript += " + (" + dataBindingExpr + ")";
			}
			// TODO: let's generalize this, to merge these 3 checks and just do plainText += token.str.substr(1);
			else if (token.type === TokenType.escapedSymbol)
			{
				// unescape the escaped symbol (removing the \)
				plainText += token.str.substr(1);
				reader.next();

			}
			else
			{
				// kinda error, but handle it gracefully and just add that to plain text
				console.error(`Unexpected token ${token.type} : ${token.str}`);
				//throw ("bad token");
				plainText += token.str;
				reader.next();
			}			
		}

		// if plaintextstart is still 0, return null (no token encountered, and no modification to the string.)
		if (plainTextStart === 0)
			return null;

		// add remainder to the plaintext
		plainText += reader.str.substring(plainTextStart);

		// if no token list, the plaintext will be the modified text and return null for binding
		if (!compiledScript)
			return [plainText, null];

		// then add plaintext to the binding list (if there is any)
		compiledScript += " + " + JSON.stringify(plainText);

		// Create and return the binding function, all original text is consumed so return the empty string as the modified.
		return ["", createDataBindingFunc(compiledScript, "\"\"")];
	}

	function parseDataBindingToScript(str)
	{
		// TODO: can this function return the required properties as well? so we can check which ones to check in binding function?
		let lexer = CreateDataBindingLexer(str);
		let expr = "";

		while (!lexer.ended())
		{
			let token = lexer.next();
			if (token.type === TokenType.identifier)
			{
				// Global var
				if (token.str[0] === '$')
				{
					expr += token.str.substr(1);
				}
				else // Data member
				{
					expr += ("data." + token.str);
				}
			}
			else if (token.type === TokenType.memberOp)
			{
				let identifier = lexer.test(TokenType.identifier);
				if (!identifier) // expecting identifier after a member token (.)
					return null;

				// consume to skip testing global or not.
				expr += token.str;
				expr += identifier.str;
			}
			else // For all other tokens, just add to the expression.
			{
				expr += token.str; 
			}
		}

		return expr;
	}

	function parseDataBindingToFunc(str, failure)
	{
		let expr = parseDataBindingToScript(str);
		if (!expr)
			return null;

		return createDataBindingFunc(expr, failure);
	}

	function createDataBindingFunc(expr, failure)
	{
		if (!expr)
			throw ("Empty expression cannot form data binding function.");

		// TODO: should the binding function have a try catch block (which may be slower)?

		let body = `if (data === null) return ${failure};
		   return (${expr});`;

		/*jshint -W054 */
		try
		{
			return new Function("data", body);	
		}
		catch (e)
		{
			console.error(`Encountered error when creating data binding function based on expression (${expr}). The error was ${e.message}`);
			return null;
		}
	}

	function parseEventListener(str)
	{
		if (str.startsWith("->"))
		{
			return function(e)
			{
				e.stopImmediatePropagation();
				e.currentTarget.dispatchEvent(
					new CustomEvent(str.substr(2), { bubbles: true, detail: this.data }));
			};
		}
		else
		{
			// TODO: if we can find all the used properties, maybe we can check to make sure all the properties exist
			// or at least we should try catch the function call.
			let script = parseDataBindingToScript(str);

			let body = `let data = this.data;
			${script};`;

			/*jshint -W054 */
			return new Function("e", body);
		}
	}

	function parseDirective(beginNode, dependencies)
	{
		// let tokens = tokenize(beginNode.data);
		// let reader = new TokenReader(tokens);

		// if (reader.test(TokenType.directivePrefix) === null)
		// 	return null;

		// let directiveToken = reader.test(TokenType.identifier);
		// if (directiveToken === null)
		// 	return null;

		// let directiveName = directiveToken.str;
		// let directiveDef = findDirectiveDefinition(directiveName);
		// if (directiveDef === null)
		// 	return null;

		let directiveDef = null;
		let directive = null;//new directiveDef.cls();

		let node = beginNode;
		//let clauseName = directiveName;
		while (node)
		{
			//let text = node.data;
			let reader = CreateDirectiveLexer(node.data); // to save some allocation maybe reuse the same reader and just assign different string to it?
			
			let directiveToken = reader.test(TokenType.directive);
			if (!directiveToken)
				return null;

			let directiveName = getDirectiveNameFromToken(directiveToken);

			if (!directive)
			{
				directiveDef = findDirectiveDefinition(directiveName);
				if (!directiveDef)
					return null;

				directive = new directiveDef.cls();
			}

			let clauseName = directiveName;
			if (clauseName === Keyword.end)
			{
				if (!reader.ended())
					node.splitText(reader.cursor);
				break;
			}

			let clauseDef = directiveDef.findClause(clauseName);
			if (!clauseDef)
				return null;

			let assignTo = directive;
			if (clauseDef.group)
			{
				assignTo = {};
				directive[clauseDef.group].push(assignTo);
			}

			// TODO: The assignTo may not be directive (look at group)
			if (!clauseDef.parseExpr(reader, assignTo))
				return null;

			if (reader.ended())
				node = node.nextSibling;
			else
				node = node.splitText(reader.cursor);				

			// TODO: The assignTo may not be directive (look at group)
			let nextNode = clauseDef.parseContent(node, assignTo, dependencies);
			if (!nextNode)
				return null;

			node = nextNode; 
		}

		let endNode = node;
		if (endNode === null || endNode === beginNode || endNode.nodeType !== Node.TEXT_NODE)
			throw ("Incorrect parsing, invalid end node, or end node is the same as begin node.");

		let parent = beginNode.parentNode;
		let removingNode = beginNode.nextSibling;
		while (removingNode !== null && removingNode !== endNode)
		{
			parent.removeChild(removingNode);
			removingNode = beginNode.nextSibling;
		}

		return directive;

		// tokenize start node.
		// skip ##
		// next token: directive name
		// parse (...)
		// if there's token left, splitText()
		// ...
		// search for mid node (e.g. ##elif ##else)
		// any node in between will belong to the sub component
		// search for ##end, any token before splitText(), then splitText() for the rest.
		// any nodes between beginNode and endNode, will be put into a component, (if there's none, that's an error)
		// new directive is created with the enclosed component
		// a <placeholder> node is created in place of all removed nodes, assign the placeholder to the directive.
	}

	function findDirectiveDefinition(name)
	{
		return directives[name] || null;
	}

	function getDirectiveNameFromToken(token)
	{
		return /[a-zA-Z]+/.exec(token.str)[0];
	}

	class DirectiveDefinition
	{
		constructor(name, config, cls)
		{
			this.name = name;
			this.cls = cls;
			this.clauses = {};

			this.setup(config);
		}

		setup(config)
		{
			let clauseDefs = [];
			for (let i = 0; i < config.clauses.length; i++) 
			{
				let clauseConfig = config.clauses[i];
				let keywords = clauseConfig.keyword.split(/\s*,\s*/);
				let clauseDef = new ClauseDefinition(clauseConfig.expression, clauseConfig.content, clauseConfig.group);
				clauseDefs.push(clauseDef);

				for (var ki = 0; ki < keywords.length; ki++) 
				{
					let keyword = keywords[ki].trim();
					if (!keyword)
					{	
						console.error(`Empty keyword found in directive clause config (${clauseConfig.keyword})`);
						continue;
					}

					if (this.clauses[keyword])
					{
						console.error(`Duplicated keyword ${keyword} found in directive clause config (${clauseConfig.keyword})`);
						continue;
					}

					this.clauses[keyword] = clauseDef;
				}
			}

			let branches = Object.keys(this.clauses);
			let mainBranchIdx = branches.indexOf(this.name);
			if (mainBranchIdx >= 0)
				branches.splice(mainBranchIdx, 1);
			else
				console.error(`No main clause found for directive ${this.name}, this directive will not be usable.`);

			for (let i = 0; i < clauseDefs.length; i++) 
			{
				let clauseDef = clauseDefs[i];
				let clauseConfig = config.clauses[i];
				if (!clauseConfig.isFinal)
					clauseDef.branches = branches;
			}
		}

		findClause(name)
		{
			return this.clauses[name] || null;
		}
	}

	var DirectiveExpressionElementType =
	{
		keyword			: 0,
		symbol			: 1,
		constant		: 2,
		binding			: 3,
	};

	class DirectiveExpressionElement
	{
		constructor(type, str)
		{
			this.type = type;
			this.str = str;
		}
	}

	class ClauseDefinition
	{
		constructor(expr, content, group)
		{
			this.content = content || "";
			this.group = group || "";
			this.elements = expr ? compileDirectiveExprElements(expr) : [];
			this.branches = [];
		}

		parseExpr(lexer, assignTo)
		{
			for (let i = 0; i < this.elements.length; i++) 
			{
				// Unexpected ending
				if (lexer.ended())
					return false;

				let element = this.elements[i];
				if (element.type === DirectiveExpressionElementType.keyword)
				{
					let token = lexer.next();

					// Wrong keyword
					if (token.str !== element.str)
						return false;

					// Wrong type
					if (token.type !== TokenType.identifier &&
						token.type !== TokenType.operator &&
						token.type !== TokenType.openParen &&
						token.type !== TokenType.closeParen)
						return false;
				}
				else if (element.type === DirectiveExpressionElementType.symbol)
				{
					let token = lexer.next();

					if (token.type !== TokenType.identifier)
						return false;

					assignTo[element.str] = token.str;
				}
				else
				{
					let nextElement = this.elements[i+1];

					// constants and data bindings must be terminated by the next keyword element.
					if (!nextElement || nextElement.type !== DirectiveExpressionElementType.keyword) 
						return false;

					let script = parseUntilTerminate(lexer, nextElement.str);
					if (!script) // empty or other invalid script causes failure.
						return false;

					if (element.type === DirectiveExpressionElementType.constant)
					{						
						try
						{
							/*jshint -W054 */
							let evaluator = new Function(`return (${script});`); 

							let constant = evaluator();
							if (constant === undefined) // should we also check the expected type? (but how do we know?, maybe we have to check it at runtime?)
								return false;

							assignTo[element.str] = constant;	
						}
						catch (e)
						{
							console.error(`Encountered error when evaluating directive const expression (${script}). The error was ${e.message}`);
							return false;
						}
					}
					else if (element.type === DirectiveExpressionElementType.binding)
					{
						let bindingFunc = parseDataBindingToFunc(script, "null");
						if (!bindingFunc)
							return false;

						assignTo[element.str] = bindingFunc;
					}
					else // unexpected element type.
					{
						return false;
					}
				}
			}

			return true;
		}

		parseContent(firstInnerNode, assignTo, dependencies)
		{
			let node = firstInnerNode;
			let depth = 0;
			while (node)
			{
				if (node.nodeType === Node.TEXT_NODE)
				{
					//let tokens = tokenize(node.data);
					let reader = CreateTextLexer(node.data);

					while (!reader.ended())
					{
						let token = reader.next();
						if (token.type === TokenType.directive)
						{
							// let directiveToken = reader.test(TokenType.identifier);
							// if (directiveToken === null)
							// 	return null;

							let directiveName = getDirectiveNameFromToken(token); //directiveToken.str;
							
							let beginNestedDirective = !!findDirectiveDefinition(directiveName);
							let endNestedDirective = (depth > 0 && directiveName === Keyword.end);

							if (beginNestedDirective)
							{
								depth++;
							}
							else if (endNestedDirective)
							{
								depth--;
							}
							else if (depth === 0)
							{
								// ending the whole clause content.
								if (directiveName === Keyword.end ||
									this.branches.includes(directiveName))
								{
									let endNode = node;

									// split out the part before
									if (token.start > 0)
										endNode = node.splitText(token.start);

									if (endNode.parentNode !== firstInnerNode.parentNode)
										return null;

									// grab everything from first to last node (node before end node) and form a new component
									let componentContent = document.createDocumentFragment();
									let transferNode = firstInnerNode;
									while (transferNode !== endNode)
									{
										// Unexpected end, technically shouldn't happen since we already checked first and end node have the same parent.
										if (!transferNode)
											return null;

										let nextNode = transferNode.nextSibling;
										componentContent.appendChild(transferNode); // no need to remove first?
										transferNode = nextNode;
									}

									if (componentContent.hasChildNodes())
									{
										// Not supposed to have content.
										if (!this.content || !assignTo)
											return null;

										let componentDef = parseComponent(componentContent, dependencies);

										// Relax this limitation.
										// Does not support sub component with more than one root node at the moment.
										//if (componentDef.content.childNodes.length > 1)
										//	return null;

										assignTo[this.content] = componentDef;
									}
									else if (this.content)
									{
										// Expecting content.
										return null;
									}

									return endNode;
								}
							}
							// else the directive is branches in other directives, skip forward.
						}
					}
				}
				
				node = node.nextSibling;
			}

			// no ##end found, fail.
			return null;
		}
	}

	function compileDirectiveExprElements(expr)
	{
		let elements = [];
		let lexer = CreateDirectiveExpressionSyntaxLexer(expr);
		let token = null;
		while ((token = lexer.next()) !== null)
		{
			let type = DirectiveExpressionElementType.keyword;
			let str = token.str;

			if (token.type === TokenType.identifier)
			{
				let intro = lexer.peek();
				if (intro && intro.type === TokenType.operator && intro.str === "%")
				{
					lexer.next();
					let typeSpecifier = lexer.test(TokenType.identifier);
					if (!typeSpecifier)
					{
						console.error(`Missing type specifier for ${str}, in expression '${expr}'`);
						return [];
					}
					else if (typeSpecifier.str === "s")
					{
						type = DirectiveExpressionElementType.symbol;
					}
					else if (typeSpecifier.str === "c")
					{
						type = DirectiveExpressionElementType.constant;
					}
					else if (typeSpecifier.str === "b")
					{
						type = DirectiveExpressionElementType.binding;
					}
					else
					{
						console.error(`Invalid type specifier '${typeSpecifier.str}' for ${str}, in expression '${expr}'`);
						return [];
					}
				}
			}
			
			elements.push(new DirectiveExpressionElement(type, str));
		}

		return elements;
	}

	function parseUntilTerminate(lexer, terminator)
	{
		let script = "";
		let depth = 0;

		while (true)
		{
			let token = lexer.peek();
			if (!token) // unexpected end
				return "";

			if (depth === 0 && token.str === terminator) // any matching string is fine, type is not really needed.
				return script;

			if (token.type === TokenType.openParen)
				depth++;
			else if (token.type === TokenType.closeParen && depth > 0)
				depth--;

			script += token.str;
			lexer.next();
		}
	}


// ███████╗██╗     ███████╗███╗   ███╗███████╗███╗   ██╗████████╗
// ██╔════╝██║     ██╔════╝████╗ ████║██╔════╝████╗  ██║╚══██╔══╝
// █████╗  ██║     █████╗  ██╔████╔██║█████╗  ██╔██╗ ██║   ██║   
// ██╔══╝  ██║     ██╔══╝  ██║╚██╔╝██║██╔══╝  ██║╚██╗██║   ██║   
// ███████╗███████╗███████╗██║ ╚═╝ ██║███████╗██║ ╚████║   ██║   
// ╚══════╝╚══════╝╚══════╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   

	function CustomElement() 
	{
		this.template = null;
		this.component = null;
		//this.bindings = []; // TODO: can this whole list be pre-compiled and shared? <- hard because the elements need to be resolved by path.
		//this.dataContext = null;
		//this.dataObserver = null;
		//this.properties = null;
		this.customDispatcher = null;

		// TODO: test
		//this.customEvents = {};
	}

	var customElemProto = CustomElement.prototype = Object.create(HTMLElement.prototype);
	customElemProto.constructor = CustomElement;

	customElemProto.setup = function(template)
	{
		this.template = template;
		this.component = new Component(template.componentDef);
		

		// TODO: do not create shadow root if the content is empty?
		// Add the template content to the shadow root
		//var clonedContent = document.importNode(templateElem.content, true);
		let shadowRoot = this.createShadowRoot();
		this.component.attach(shadowRoot, null, this);
		//shadowRoot.appendChild(clonedContent);

		// TODO: simplify (do we still need this?)
		var templateElem = template.element;
		var numClasses = templateElem.classList.length;
		for (var i = 0; i < numClasses; i++) 
		{
			this.classList.add(templateElem.classList.item(i));
		}

		// TODO: may want to inline to improve perf.
		//attachBehaviors(this);
		//initBindings(this);

		// TODO: maybe we should only do these when attached
		// and only setup the binding observer after attached.
		// we can still accept any dataContext when bind() is called.
		// if we do that make sure we clear that everytime (or on detach),
		// because the attached callback may occur multiple times.

		// var infos = this.template.elementInfos;
		// var ni = infos.length;
		// for (i = 0; i < ni; i++) 
		// {
		// 	var info = infos[i];
		// 	var element = traverseSubElement(this, info.path);

		// 	if (info.bindingFunc !== null || info.propBindings.length > 0)
		// 	{
		// 		var binding = new Binding();
		// 		binding.element = element;
		// 		binding.func = info.bindingFunc;
		// 		binding.propBindings = info.propBindings;
		// 		this.bindings.push(binding);
		// 	}

		// 	// TODO: need double check this is custom element?
		// 	for (var p = 0; p < info.properties.length; p++) 
		// 	{
		// 		var prop = info.properties[p];
		// 		prop.property.set(element, prop.value);   		
		// 	}

		// 	for (var ei = 0; ei < info.eventBindings.length; ei++) 
		// 	{
		// 		var eventBinding = info.eventBindings[ei];
		// 		// TODO: bind() is slow, avoid using or at least replace it with a simpler version.
		// 		element.addEventListener(eventBinding.type, eventBinding.listener.bind(this));
		// 	}

		// 	// Lastly attach the behaviors, so they won't accidently receive the property set event,
		// 	// They are supposed to init using onSetup event, and then start listening to onCustomPropertyChanged
		// 	for (var b = 0; b < info.behaviors.length; b++) 
		// 	{
		// 		info.behaviors[b].attached(element);
		// 	}
		// }

		//this.bind(null);
	};

	customElemProto.bind = function(key, value)
	{
		this.component.setDataProp(key, value);
		this.component.refresh();

		// var oldData = this.dataContext;

		// if (oldData === data)
		// 	return;

		// observeData(this, data);
		// refreshBindings(this);

		//this.dispatchEvent(new CustomEvent("bindingchanged", { detail: { previous: oldData, current: data } }));

		// if (this.customEvents.bindingchanged)
		// 	this.customEvents.bindingchanged({ currentTarget:this, detail: { previous: oldData, current: data } });
		//this.dispatchCustomEvent(new BindingChanged(oldData, data));
	};

	customElemProto.refresh = function()
	{
		this.component.refresh();
	};

	customElemProto.addCustomEventListener = function(type, handler)
	{
		if (this.customDispatcher === null)
			this.customDispatcher = new EventDispatcher();

		this.customDispatcher.addListener(type, handler, null);
	};

	customElemProto.dispatchCustomEvent = function(e)
	{
		if (this.customDispatcher !== null)
		{
			e.currentTarget = this;
			this.customDispatcher.dispatch(e);
		}
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

	/*function refreshBindings(element)
	{
		var data = element.dataContext;
		var numBindings = element.bindings.length;
		for (var i = 0; i < numBindings; i++) 
		{
			element.bindings[i].apply(data);
		}

		// pass to behavior
		//element.dispatchEvent(new CustomEvent("datachanged", { detail: { data: data } }));	
		// if (element.customEvents.datachanged)
		// 	element.customEvents.datachanged({ currentTarget: element, detail: { data: data } });

		element.dispatchCustomEvent(new DataChanged(data));

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
	}

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
		}
		return cursor;
	}

	// var BindingInfo = Class(
	// {
	// 	constructor : function(path, func)
	// 	{
	// 		this.path = path.concat();
	// 		this.func = func;
	// 	}
	// });

	var Binding = Class(
	{
		constructor : function()
		{
			this.element = null;
			this.func = null;
			this.propBindings = null;
		},

		apply : function(data)
		{
			var value;

			var l = this.propBindings.length;
			for (var i = 0; i < l; i++) 
			{
				var binding = this.propBindings[i];
				value = binding.func(data); // TODO: should try catch
				binding.property.set(this.element, value);
			}

			if (this.func !== null)
			{
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
					this.element.textContent = html; // TODO: maybe check === first? setting html content normally fairly expensive.
				}				
			}
		}
	});*/



// 	███████╗██╗   ██╗███████╗███╗   ██╗████████╗███████╗
// 	██╔════╝██║   ██║██╔════╝████╗  ██║╚══██╔══╝██╔════╝
// 	█████╗  ██║   ██║█████╗  ██╔██╗ ██║   ██║   ███████╗
// 	██╔══╝  ╚██╗ ██╔╝██╔══╝  ██║╚██╗██║   ██║   ╚════██║
// 	███████╗ ╚████╔╝ ███████╗██║ ╚████║   ██║   ███████║
// 	╚══════╝  ╚═══╝  ╚══════╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝


	function registerCustomEvent(eventConstructor)
	{
		var name = eventConstructor.name;
		if (customEvents[name])
			throw ("duplicated event name.");

		customEvents[name] = eventConstructor;
	}

	class DataChanged
	{
		constructor(data)
		{
			this.data = data;
		}
	}
	registerCustomEvent(DataChanged);

	class BindingChanged
	{
		constructor(previous, current)
		{
			this.previous = previous;
			this.current = current;
		}
	}
	registerCustomEvent(BindingChanged);

	class ElementAttached
	{
		constructor()
		{
		}
	}
	registerCustomEvent(ElementAttached);





// ██████╗ ███████╗██╗  ██╗ █████╗ ██╗   ██╗██╗ ██████╗ ██████╗ 
// ██╔══██╗██╔════╝██║  ██║██╔══██╗██║   ██║██║██╔═══██╗██╔══██╗
// ██████╔╝█████╗  ███████║███████║██║   ██║██║██║   ██║██████╔╝
// ██╔══██╗██╔══╝  ██╔══██║██╔══██║╚██╗ ██╔╝██║██║   ██║██╔══██╗
// ██████╔╝███████╗██║  ██║██║  ██║ ╚████╔╝ ██║╚██████╔╝██║  ██║
// ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝  ╚═══╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝

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
						var eventName = key.substr(2);
						var customEventType = customEvents[eventName];						
						if (customEventType !== undefined)
							this.listeners.push(new BehaviorCustomEventListener(this, customEventType , value));
						else
							this.listeners.push(new BehaviorListener(this, eventName, value));
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
			}
		},

		init: function(element)
		{			
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
				//var listener = this.listeners[i];
				//element.addEventListener(listener.type, listener.handler);
				//element.customEvents[listener.type] = listener.handler;
				this.listeners[i].attachTo(element);
			}

			this.init(element);
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
		},

		attachTo: function(element)
		{
			element.addEventListener(this.type, this.handler);
		}
	});

	var BehaviorCustomEventListener = Class(
	{
		constructor : function BehaviorCustomEventListener(behavior, type, handler)
		{
			this.type = type;
			this.handler = function(event)
			{
				handler.call(behavior, event.currentTarget, event);
			};
		},

		attachTo: function(element)
		{
			element.addCustomEventListener(this.type, this.handler);
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
	}


// ██████╗ ██████╗  ██████╗ ██████╗ ███████╗██████╗ ████████╗██╗   ██╗
// ██╔══██╗██╔══██╗██╔═══██╗██╔══██╗██╔════╝██╔══██╗╚══██╔══╝╚██╗ ██╔╝
// ██████╔╝██████╔╝██║   ██║██████╔╝█████╗  ██████╔╝   ██║    ╚████╔╝ 
// ██╔═══╝ ██╔══██╗██║   ██║██╔═══╝ ██╔══╝  ██╔══██╗   ██║     ╚██╔╝  
// ██║     ██║  ██║╚██████╔╝██║     ███████╗██║  ██║   ██║      ██║   
// ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝      ╚═╝   


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

	var ObjectProperty = Class(CustomProperty,
	{
		constructor : function ObjectProperty(name, readonly)
		{
			ObjectProperty.$super.call(this, name, readonly);
		},

		parse : function(str)
		{
			throw ("NOT IMPLEMENTED!");
		},

		stringify : function(value)
		{
			throw ("NOT IMPLEMENTED!");
		},

		coerce : function(value)
		{
			if (typeof value !== 'object')
				return null;
			return value;
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
		else if (type === Object)
		{
			prop = new ObjectProperty(name, readonly);
		}
		else
		{
			throw (`unknown property type ${type}`);
		}
		properties[name] = prop;
		return prop;
	}

	function findProperty(name)
	{
		return properties[name] || null;
	}

	function attrToPropName(attrName)
	{
		return attrName.trim().replace(/[\s-_]+([a-zA-Z])/g, 
			function(match, p1) { return p1.toUpperCase(); });
	}



// 	██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ ███████╗
// 	██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗██╔════╝
// 	███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝███████╗
// 	██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗╚════██║
// 	██║  ██║███████╗███████╗██║     ███████╗██║  ██║███████║
// 	╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝

	function calcElementOffset(element, reference)
	{
		// var toRect = element.getBoundingClientRect();
		// var fromRect = reference.getBoundingClientRect();
		// var left = toRect.left - fromRect.left;
		// var top = toRect.top - fromRect.top;
		return calcPageOffset(element).sub(calcPageOffset(reference));
	}	

	function calcPageOffset(element)
	{
		var current = element;
		var parent;
		var left = 0;
		var top = 0;
		while ((parent = current.offsetParent) !== null)
		{
			left += current.offsetLeft;
			top += current.offsetTop;
			current = parent;
		}
		return new Vector2(left, top);
	}
	

})(this);























/*




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

		var tokens = header.split(/\s*#\s* /);asdfsadf wrong shit remove the space
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
	
})(this);*/


