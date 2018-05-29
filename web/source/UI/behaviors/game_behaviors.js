/*global PanelAddedEvent: false*/

UI.Behavior("player-view", 
{
	onBindingChanged: function(element, event)
	{
		var player = event.current;
		player.events.addListener(PanelAddedEvent, onPanelAdded, null);

		function onPanelAdded(e)
		{
			if (e.fromAction !== null)
			{
				var actionList = element.shadowRoot.querySelector("action-list");
				var l = actionList.children.length;
				var item = null;
				for (var i = 0; i < l; i++) 
				{
					var actionItem = actionList.children[i].children[0];
					if (actionItem.dataContext === e.fromAction)
					{
						item = actionList.children[i]; // do the container for now.
						break;
					}
				}

				var foreground = element.shadowRoot.querySelector("#foreground");
				if (item !== null)
				{
					var offset = UI.calcElementOffset(item, foreground);
					foreground.appendChild(item);
					item.style.position = "absolute";
					item.style.left = offset.x + "px";
					item.style.top = offset.y + "px";
					item.style.removeProperty("right");
				}
			}
		}
	}
});

UI.Behavior("action-list", 
{
	actionContainerTemplate: { type: String, value: "", readonly: true },
	actionItemTemplate: { type: String, value: "", readonly: true },
	actionOffsetX: { type: Number, value: 0, readonly: false },
	actionOffsetY: { type: Number, value: 0, readonly: false },

	onDataChanged: function(element, event)
	{
		var containerTemplate = this.actionContainerTemplate.get(element);
		var itemTemplate = this.actionItemTemplate.get(element);

		if (!containerTemplate || !itemTemplate)
		{
			console.error(`No proper action container and item template defined: ${containerTemplate}, ${itemTemplate}`);
			return;
		}			

		var list = event.data;

		if (list !== null && !Array.isArray(list))
		{
			console.error(`Using collection behavior but bound to non-array data: ${JSON.stringify(list)}`);
			list = null;
		}

		var l = list !== null ? list.length : 0;
		var item;
		var i;

		var positions = [];
		positions.length = l;
		var pos;

		function ItemPositionInfo()
		{
			this.x = 0;
			this.y = 0;
			this.dx = 0;
			this.dy = 0;
			this.width = 0;
			this.height = 0;
		}

		if (list !== null)
		{
			for (i = element.children.length - 1; i >= 0; i--) 
			{
				item = element.children[i].children[0];
				var action = item.dataContext;
				var existingIdx = list.indexOf(action);
				if (existingIdx >= 0)
				{
					pos = new ItemPositionInfo();
					pos.dx = this.actionOffsetX.get(item);
					pos.dy = this.actionOffsetY.get(item);
					positions[existingIdx] = pos;
				}
			}
		}


		for (i = element.children.length - 1; i >= l; i--) 
		{
			element.removeChild(element.children[i]);
		}

		var container;
		if (list !== null)
		{
			for (i = element.children.length; i < l; i++) 
			{
				container = document.createElement(containerTemplate);
				item = document.createElement(itemTemplate);
				container.appendChild(item);
				element.appendChild(container);
			}
		}

		l = element.children.length;
		if (l > 0)
		{
			for (i = 0; i < element.children.length; i++) 
			{
				container = element.children[i];
				item = container.children[0];
				var entry = list[i];
				container.bind(entry);
				item.bind(entry);
			}

			var containerHeight = element.clientHeight;
			var actionsHeight = 0;

			for (i = 0; i < element.children.length; i++) 
			{
				item = element.children[i].children[0];

				if (positions[i] === undefined)
					positions[i] = new ItemPositionInfo();

				pos = positions[i];
				pos.width = item.offsetWidth;
				pos.height = item.offsetHeight;

				actionsHeight += pos.height;
			}

			var maxY = Math.min((containerHeight - actionsHeight) / 2, 64);

			var top = 0;
			for (i = 0; i < element.children.length; i++) 
			{
				container = element.children[i];
				//item = container.children[0];
				pos = positions[i];
				
				top += MathEx.randomInt(maxY - 48, maxY);

				container.style.position = "absolute";
				container.style.top = top + "px";
				container.style.right = "0px";
				top += pos.height;
			}			
		}

	}
});