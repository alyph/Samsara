var Sprite = Class(
{
	constructor : function (parent, sprite, id, class) 
	{
		this.parent = parent;
		this.sprite = sprite;

		this.image = ($("<div class='spriteElement' />"));

		if (id)
			this.image.attr("id", id);

		if (class)
			this.image.addClass(class);

		this.image.css("background-image", "url("+sprite.imageURL+")");

		this.image.appendTo(parent);
		this.updateBackground();

		this.image.resize({ elem : this }, this.resized);
	}

	resized : function(evt)
	{
		evt.data.elem.updateBackground();
	}

	updateBackground : function()
	{
		var image = this.image;
		var sprite = this.sprite;
		var iw = image.width();
		var ih = image.height();
		var backSize = Math.round(sprite.domO.naturalWidth / sprite.delta) * iw;
		var offsetX = -Math.round(sprite.offsetx / sprite.delta) * iw;
		var offsetY = -Math.round(sprite.offsety / sprite.distance) * ih;

		image.css(
			{
				"background-size" : backSize + "px",
				"background-position" : offsetX + "px " + offsetY + "px"
			});
	}
});


