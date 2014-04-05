var Board = Class(
{
	constructor : function (table) 
	{
		this.table = table;
		this.node = $("<div style='width: 100%; height: 100%;'></div>").appendTo(table.node);
		this.hide();
	},

	show : function()
	{
		this.node.show();
	},

	hide : function()
	{
		this.node.hide();
	}
});

var LocationBoard = Class(Board,
{
	constructor : function(table)
	{
		LocationBoard.$super.call(this, table);

		var w = 200;
		var h = 100;
		var x = Math.floor((this.node.width() - w) / 2);
		var y = Math.floor((this.node.height() - h) / 2);

		//this.background = new Picture(this.node, x, y, w, h);
	},

	setLocation : function(loc)
	{
		//this.background.show(Sprites[loc.def.background]);

		$(".activity").remove();

		var locInfo = $("<div class='location'/>").appendTo(this.node);
		var locBack = new Picture(locInfo, 0, 0, 200, 100);
		locBack.show(Sprites[loc.def.background]);


		this.addEntity("Market", ["Visit the Bazzar"]);
		this.addEntity("Library", ["Research the symbol"]);
		/*
		for (var i = loc.activities.length - 1; i >= 0; i--) 
		{
			this.addActivityNode(loc.activities[i]);
		};*/
	},

	addEntity : function(back, options)
	{
		var entity = $("<div class='entity' />").appendTo(this.node);
		var entityBack = new Picture(entity, 0, 0, 48, 64);
		entityBack.show(Sprites[back]);

		var opt = $("<ul class='actions'/>").appendTo(entity);

		for (var i = options.length - 1; i >= 0; i--) 
		{
			opt.append("<li>" + options[i] + "</li>");
		};

	},

	addActivityNode : function(activity)
	{
		var node = $("<div class='activity'>" + activity.getDesc() + "</div>").appendTo(this.node);
		var x = MathEx.randomInt(64, this.table.width - 64);
		var y = MathEx.randomInt(64, this.table.height - 64);
		node.css(
		{
			"position": "absolute",
			"left" : "" + x + "px",
			"top" : "" + y + "px"
		});
	}
});

