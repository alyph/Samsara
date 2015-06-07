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

UI.SceneViewer = Class(UI.ItemContainer,
{
	constructor : function()
	{
		UI.SceneViewer.$super.call(this);
	},

	getList : function()
	{
		return this.data.pois;
	},

	rearrange : function()
	{
		var l = this.numVisible;
		if (l === 0)
			return;

		var w = this.width();
		var h = this.height();

		var numRows = Math.floor(Math.sqrt(l));
		var numCols = Math.ceil(l / numRows);

		var padding = 32;
		var totalWidth = 0;
		var totalHeight = padding;
		var iws = []; iws.length = l;
		var ihs = []; ihs.length = l;
		var rw = []; rw.length = numRows;
		var rh = []; rh.length = numRows;

		for (var r = 0; r < numRows; r++) 
		{
			rw[r] = padding; 
			rh[r] = 0;

			for (var c = 0; c < numCols; c++)
			{
				var i = r * numCols + c;
				if (i >= l) 
					break;

				var iw = this.items[i].width();
				var ih = this.items[i].height();
				iws[i] = iw;
				ihs[i] = ih;
				rw[r] += iw + padding;
				rh[r] = Math.max(rh[r], ih);
			};

			totalWidth = Math.max(totalWidth, rw[r]);
			totalHeight += rh[r] + padding;
		};

		var y = Math.floor((h - totalHeight) / 2);
		for (var r = 0; r < numRows; r++) 
		{
			var x = Math.floor((w - rw[r]) / 2);
			for (var c = 0; c < numCols; c++)
			{
				var i = r * numCols + c;
				if (i >= l) 
					break;

				this.items[i].position(x, y);

				x += iws[i] + padding;
			};

			y += rh[r] + padding;
		};
	}
});

