UI.Board = Class(UI.Element,
{
	constructor : function()
	{
		UI.Board.$super.call(this);
	}
});

UI.StagePanel = Class(UI.Element,
{
	constructor : function()
	{
		UI.StagePanel.$super.call(this);

		this.actors = [];
		this.actorTemplate = null;
		this.gridSize = 32;
	},

	setup : function(props)
	{
		UI.StagePanel.$superp.setup.call(this, props);
		
		this.actorTemplate = props.actorTemplate;
		if (!this.actorTemplate)
			throw ("must define actor template!");
	},

	refresh : function()
	{
		UI.StagePanel.$superp.refresh.call(this);
		
		this.clearStage();
		
		if (this.data) 
			this.initStage();
	},

	clearStage : function()
	{
		this.actors.length = 0;
	},

	initStage : function()
	{
		var stage = this.data;
		var actors = stage.grid.objects;

		for (var i = actors.length - 1; i >= 0; i--) 
		{
			data = actors[i];
			var actorItem = this.createActorItem();
			actorItem.setData(data.object);
			this.append(actorItem);
			this.positionActor(actorItem);
			
		};
	},

	createActorItem : function()
	{
		var newItem = this.actorTemplate.instance();
		newItem.css("position", "absolute");
		return newItem;
	},

	positionActor : function(actorItem)
	{
		var actor = actorItem.data;
		var actorX = actor.x();
		var actorY = actor.y();
		var actorW = actor.width;
		var actorH = actor.height;

		var s = this.gridSize;
		var bottom = Math.floor((actorY + actorH) * s);
		var x = Math.floor(actorX * s + (actorW * s - actorItem.width()) / 2);
		var y = bottom - actorItem.height();
		actorItem.position(x, y, bottom);
	}
});

