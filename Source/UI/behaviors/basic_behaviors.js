UI.Behavior("collection", 
{
	itemTemplate: { type: String, value: "", readonly: true },
	itemTemplates: { type: String, value: "", readonly: true },
	classToTemplateMap: { type: Object, value: null, readonly: false },

	onDataChanged: function(element, event)
	{
		var list = event.data;

		if (list !== null && !Array.isArray(list))
		{
			console.error(`Using collection behavior but bound to non-array data: ${JSON.stringify(list)}`);
			list = null;
		}

		if (list !== null)
		{
			this.initTemplateMap(element);
		}

		var l = list !== null ? list.length : 0;
		for (var i = element.children.length - 1; i >= l; i--) 
		{
			element.removeChild(element.children[i]);
		}

		if (list !== null)
		{
			for (i = element.children.length; i < l; i++) 
			{
				var template = this.getItemTemplate(element, list[i]);
				if (template)
				{
					element.appendChild(document.createElement(template));
				}
				else
				{
					console.error("No item template specified.");
				}
			}			
		}

		l = element.children.length;
		for (i = 0; i < element.children.length; i++) 
		{
			element.children[i].bind(list[i]);
		}
	},

	initTemplateMap: function(element)
	{
		var map = this.classToTemplateMap.get(element);
		var templates = this.itemTemplates.get(element);
		if (!map && templates)
		{
			map = new Map();
			var templateNames = templates.split(/\s+/);
			for (var i = 0; i < templateNames.length; i++) 
			{
				var name = templateNames[i];
				var template = UI.findTemplate(name);
				if (!template)
				{
					console.error(`the template ${name} in itemTemplates list does not exist!`);
					continue;
				}

				if (!template.dataClass)
				{
					console.error(`the template ${name} has no data class!`);
					continue;
				}

				map.set(template.dataClass, name);
			}

			this.classToTemplateMap.set(element, map);
		}
	},

	getItemTemplate: function(element, data)
	{
		var map = this.classToTemplateMap.get(element);
		if (map !== null)
		{
			return map.get(data.constructor.name);
		}
		else
		{
			return this.itemTemplate.get(element);
		}
	}
});

UI.Behavior("sprite", 
{
	onBindingChanged: function(element, event)
	{
		// var prevSprite = this.getSprite(event.detail.previous);
		var currSprite = this.getSprite(event.current);

		// if (prevSprite !== null)
		// 	element.classList.remove(prevSprite.image.className);

		if (currSprite !== null)
		{
			var img = currSprite.image;

			// element.classList.add(currSprite.image.className);
			element.style.backgroundImage = "url(" + img.url + ")";	
			element.style.backgroundSize = img.tilesx + "00% " + img.tilesy + "00%";
			element.style.backgroundPosition = -currSprite.x + "00% " + -currSprite.y + "00%";

			// if (currSprite.width > 0 && currSprite.height > 0)
			// {
			// 	//var iw = element.clientWidth;
			// 	//var ih = element.clientHeight;

			// 	var backSize = Math.round(currSprite.image.DOM.naturalWidth / currSprite.width) * iw;
			// 	var offsetX = -Math.round(currSprite.offsetx / currSprite.width) * iw;
			// 	var offsetY = -Math.round(currSprite.offsety / currSprite.height) * ih;	
			// 	element.style.backgroundSize = backSize + "px";
			// 	element.style.backgroundPosition = offsetX + "px " + offsetY + "px";
			// 	//cssProps["background-size"] = backSize + "px";
			// 	//cssProps["background-position"] = offsetX + "px " + offsetY + "px";
			// }
			// else
			// {
			// 	element.style.backgroundSize = element.clientWidth + "px";
			// }
		}
		else
		{
			element.style.removeProperty("background-image");
		}
	},

	getSprite: function(data)
	{
		if (data && data.image && data.image.className && data.image.DOM)
		{
			return data;
		}

		return null;
	}
});

UI.Behavior("scale", 
{
	scale: { type: Number, value: 1, readonly: false },
	//originalWidth: { type: Number, value: 1, readonly: false },
	//originalHeight: { type: Number, value: 1, readonly: false },

	onElementAttached: function(element)
	{
		//this.originalWidth.set(element, element.clientWidth);
		//this.originalHeight.set(element, element.clientHeight);
		this.updateScale(element, this.scale.get(element));
	},

	updateScale: function(element, scale)
	{
		var scaledStyle = Math.round(scale * 100) + "%";
		element.style.width = scaledStyle;
		element.style.height = scaledStyle;


		// var ow = this.originalWidth.get(element);
		// var oh = this.originalHeight.get(element);
		// var nw = Math.round(ow * scale);
		// var nh = Math.round(oh * scale);
		// element.style.width = nw + "px";
		// element.style.height = nh + "px";
	}
});
