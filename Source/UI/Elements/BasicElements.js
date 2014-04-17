UI.Label = Class(UI.Element,
{
	constructor : function()
	{
		UI.Label.$super.call(this);
	},

	refresh : function()
	{
		var text = this.data || "";
		this.DOM.html(text);
	}
});

UI.Image = Class(UI.Element,
{
	constructor : function()
	{
		UI.Image.$super.call(this);
	},

	refresh : function()
	{
		// TODO: when size changed, must refresh()
		if (this.data)
		{
			var cssProps = {};
			var sprite = this.data;

			if (!sprite.imageURL)
				throw ("invalid image data!");

			cssProps["background-image"] = "url("+sprite.imageURL+")";

			if (sprite.delta > 0 && sprite.distance > 0)
			{
				var iw = this.width();
				var ih = this.height();
				var backSize = Math.round(sprite.domO.naturalWidth / sprite.delta) * iw;
				var offsetX = -Math.round(sprite.offsetx / sprite.delta) * iw;
				var offsetY = -Math.round(sprite.offsety / sprite.distance) * ih;	
				cssProps["background-size"] = backSize + "px";
				cssProps["background-position"] = offsetX + "px " + offsetY + "px";
			}

			this.css(cssProps);
		}
		else
		{
			this.css("background-image", "");
		}
	}
});