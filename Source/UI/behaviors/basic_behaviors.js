NewUI.Behavior("collection", 
{
	itemTemplate: { type: String, value: "", readonly: true },

	ondatachanged: function(element, event)
	{
		var list = event.detail.data;

		if (list !== null && !Array.isArray(list))
		{
			console.error(`Using collection behavior but bound to non-array data: ${JSON.stringify(list)}`);
			list = null;
		}

		var l = list !== null ? list.length : 0;
		for (var i = element.children.length - 1; i >= l; i--) 
		{
			element.removeChild(element.children[i]);
		};

		if (list !== null)
		{
			var template = this.itemTemplate.get(element);
			if (template)
			{
				for (var i = element.children.length; i < l; i++) 
				{
					element.appendChild(document.createElement(template));
				};
			}
			else
			{
				console.error("No item template specified.");
			}
		}

		l = element.children.length;
		for (var i = 0; i < element.children.length; i++) 
		{
			element.children[i].bind(list[i]);
		};
	}
});

NewUI.Behavior("sprite", 
{
	onbindingchanged: function(element, event)
	{
		// var prevSprite = this.getSprite(event.detail.previous);
		var currSprite = this.getSprite(event.detail.current);

		// if (prevSprite !== null)
		// 	element.classList.remove(prevSprite.image.className);

		if (currSprite !== null)
		{
			// element.classList.add(currSprite.image.className);
			element.style.backgroundImage = "url(" + currSprite.image.url + ")";
	
			if (currSprite.width > 0 && currSprite.height > 0)
			{
				var iw = element.offsetWidth;
				var ih = element.offsetHeight;

				var backSize = Math.round(currSprite.image.DOM.naturalWidth / currSprite.width) * iw;
				var offsetX = -Math.round(currSprite.offsetx / currSprite.width) * iw;
				var offsetY = -Math.round(currSprite.offsety / currSprite.height) * ih;	
				element.style.backgroundSize = backSize + "px";
				element.style.backgroundPosition = offsetX + "px " + offsetY + "px";
				//cssProps["background-size"] = backSize + "px";
				//cssProps["background-position"] = offsetX + "px " + offsetY + "px";
			}
			else
			{
				element.style.backgroundSize = element.offsetWidth + "px";
			}
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
