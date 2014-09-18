UI.Label = Class(UI.Element,
{
	constructor : function()
	{
		UI.Label.$super.call(this);
		this.image = null;
	},

	refresh : function()
	{
		var text = this.data || "";
		this.html(text);
	}
});

UI.Image = Class(UI.Element,
{
	constructor : function()
	{
		UI.Image.$super.call(this);

		this.useOriginalSize = false;
	},

	setup : function(props)
	{
		UI.Image.$superp.setup.call(this, props);
		
		this.useOriginalSize = props.useOriginalSize || false;
	},

	refresh : function()
	{
		// TODO: when size changed, must refresh()
		if (this.data)
		{
			var cssProps = {};
			var sprite = this.data;

			if (sprite.image !== this.image)
			{
				if (this.image)
					this.removeClass(this.image.className);

				this.addClass(sprite.image.className);
				this.image = sprite.image;
			}

			var iw = 0;
			var ih = 0;

			if (sprite.width > 0 && sprite.height > 0)
			{
				if (this.useOriginalSize)
				{
					iw = sprite.width;
					ih = sprite.height;
					this.width(iw);
					this.height(ih);
				}
				else
				{
					iw = this.width();
					ih = this.height();
				}

				var backSize = Math.round(sprite.image.DOM.naturalWidth / sprite.width) * iw;
				var offsetX = -Math.round(sprite.offsetx / sprite.width) * iw;
				var offsetY = -Math.round(sprite.offsety / sprite.height) * ih;	
				cssProps["background-size"] = backSize + "px";
				cssProps["background-position"] = offsetX + "px " + offsetY + "px";
			}
			else
			{
				if (this.useOriginalSize)
				{
					iw = sprite.image.DOM.naturalWidth;
					ih = sprite.image.DOM.naturalHeight;
					this.width(iw);
					this.height(ih);
				}
				else
				{
					iw = this.width();
				}

				cssProps["background-size"] = iw + "px";
			}

			this.css(cssProps);
		}
		else if (this.image)
		{
			this.removeClass(this.image.className);
			this.image = null;
		}
	}
});

UI.ItemContainer = Class(UI.Element,
{
	constructor : function()
	{
		UI.ItemContainer.$super.call(this);

		this.items = [];
		this.numVisible = 0;
		this.itemTemplate = null;
	},

	setup : function(props)
	{
		UI.ItemContainer.$superp.setup.call(this, props);
		
		this.itemTemplate = props.itemTemplate;
		if (!this.itemTemplate)
			throw ("must define item template!");
	},

	refresh : function()
	{
		if (this.data !== null && this.data.length > 0)
		{	
			var l = this.data.length;

			for (var i = this.items.length; i < l; i++) 
			{
				this.items.push(this.createItem());
			};

			for (var i = this.numVisible; i < l; i++) 
			{
				//this.items[i].show();
				this.append(this.items[i]);
			};

			for (var i = l; i < this.numVisible; i++) 
			{
				//this.items[i].$DOM.hide();
				this.items[i].detach();
			};

			for (var i = 0; i < l; i++) 
			{
				this.items[i].setData(this.data[i]);
			};

			this.numVisible = l;
			this.rearrange();
		}		
		else
		{
			for (var i = this.numVisible - 1; i >= 0; i--) 
			{
				this.items[i].detach();
			};
			this.numVisible = 0;
		}
	},

	rearrange : function()
	{
		throw ("must override.");
	},

	createItem : function()
	{
		var newItem = this.itemTemplate.instance();
		newItem.css("position", "absolute");
		return newItem;
	},

	handleEvent : function(e, handler, obj)
	{
		e.targetItem = this.findTargetItem(e);
		UI.ItemContainer.$superp.handleEvent.call(this, e, handler, obj);
	},

	findTargetItem : function(e)
	{
		var current = e.target;
		var $iter = $(current);
		while (current && current.$elem !== this.DOM)
		{
			$iter = $iter.parent();
			if (!$iter)
				throw ("cannot find proper element in chain!");

			var parent = $iter[0];
			if (parent === this.DOM)
			{
				return current.$elem;
			}

			current = parent;
		}

		return null;
	}
});

UI.List = Class(UI.ItemContainer,
{
	constructor : function()
	{
		UI.List.$super.call(this);
	},

	rearrange : function()
	{
		var w = this.width();
		var h = this.height();
		var l = this.numVisible;

		var totalWidth = 0;
		var iw = [];
		var ih = [];
		for (var i = 0; i < l; i++) 
		{
			iw.push(this.items[i].width());
			ih.push(this.items[i].height())
			totalWidth += iw[i]
		};

		var padding = l > 1 ? Math.min((w - totalWidth) / (l - 1), 64) : 0;
		var left = 0;//Math.floor((w - totalWidth) / 2);
		var rand = 16;

		for (var i = 0; i < l; i++) 
		{
			var top = Math.floor((h - ih[i]) / 2);
			this.items[i].position(left, MathEx.randomInt(top - rand, top + rand));
			left += iw[i] + padding;
		};
	}
});





