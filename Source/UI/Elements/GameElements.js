UI.Board = Class(UI.Element,
{
	constructor : function()
	{
		UI.Board.$super.call(this);

		this.items = [];
		this.numVisible = 0;
		this.itemTemplate = null;
	},

	setup : function(props)
	{
		UI.Board.$superp.setup.call(this, props);
		
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
				this.append(this.items[i]);
			};

			for (var i = l; i < this.numVisible; i++) 
			{
				this.items[i].detach();
			};

			for (var i = 0; i < l; i++) 
			{
				this.items[i].setData(this.data[i]);
			};

			if (l !== this.numVisible)
			{
				this.numVisible = l;

				var iw = this.items[0].width();
				var ih = this.items[0].height();
				var w = this.width();
				var h = this.height();

				var padding = 64;
				var totalWidth = Math.min(l * iw + (l - 1) * padding, w - 64);
				var left = Math.floor((w - totalWidth) / 2);
				var space =  l > 1 ? (totalWidth - iw) / (l - 1) : 0;
				var top = Math.floor((h - ih) / 2);
				var rand = 16;

				for (var i = 0; i < l; i++) 
				{
					this.items[i].position(left + space * i, MathEx.randomInt(top - rand, top + rand));
				};
			}
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

	createItem : function()
	{
		var newItem = this.itemTemplate.instance();
		newItem.css("position", "absolute");
		return newItem;
	}
});