var Stage = Class(
{
	constructor : function()
	{
		this.width = 32;
		this.height = 15;
		this.grid = new Grid();
	}, 

	placeFormation : function(formation, rules)
	{
		// remove actors from the grid
		this.removeFromGrid(formation.actors);

		var margin = 2;

		// find desired location
		var desiredLoc = this.calcDesiredLoc(formation, rules, margin);

		// search for best location
		var loc = this.findBestLoc(formation, rules, desiredLoc);

		loc[0] += margin;
		loc[1] += margin;

		// place the formation's grid of actors at the given location, also update actor's location
		this.placeGrid(formation.grid, loc);
	},

	removeFromGrid : function(actors)
	{
		for (var i = actors.length - 1; i >= 0; i--) 
		{
			if (actors[i].isOnStage())
				this.grid.remove(actors[i]);
		};
	},

	placeGrid : function(grid, loc)
	{
		var actors = grid.objects;
		var dx = loc[0] - grid.left();
		var dy = loc[1] - grid.top();

		for (var i = actors.length - 1; i >= 0; i--) 
		{
			var data = actors[i];
			var actor = data.object;
			var loc = data.loc.concat();
			loc[0] += dx;
			loc[1] += dy;
			actor.loc = loc;
			actor.facing = data.facing || "left";
			this.grid.place(actor, loc);
		};

		// notify actors moved event
		//throw ("not implemented!");
	},

	calcDesiredLoc : function(formation, rules, margin)
	{
		var w = formation.grid.width() + margin * 2;
		var h = formation.grid.height() + margin * 2;

		if (w > this.width || h > this.height)
		{
			throw ("formation is too big cannot fit!");
		}

		var x = 0;
		var y = 0;

		if (!rules.anchor)
		{
			x = MathEx.randomInt(0, this.width - w);
			y = MathEx.randomInt(0, this.height - h);
		}
		else if (rules.anchor === "left")
		{
			x = 0;
			y = Math.floor((this.height - h) / 2);
		}
		else
		{
			x = this.width - w;
			y = Math.floor((this.height - h) / 2);	
		}

		return [x, y];
	},

	findBestLoc : function(formation, rules, desiredLoc, margin)
	{
		var left = 0;
		var top = 0;
		var right = this.width;
		var bottom = this.height;

		var partitionSize = 4;
		var partitionW = Math.ceil(this.width / partitionSize);
		var partitionH = Math.ceil(this.height / partitionSize);

		var partitions = [];

		for (var px = partitionW - 1; px >= 0; px--) 
		{
			var column = [];
			partitions[px] = column;
			for (var py = partitionH - 1; py >= 0; py--) 
			{
				column[py] = [];
			};
		};

		var formX = desiredLoc[0];
		var formY = desiredLoc[1];
		var formW = formation.grid.width();
		var formH = formation.grid.height();

		var actorData = this.grid.objects;
		for (var i = actorData.length - 1; i >= 0; i--) 
		{
			var data = actorData[i];
			var x = data.loc[0];
			var y = data.loc[1];
			var w = data.object.width;
			var h = data.object.height;

			var minX = Math.floor(Math.max(0, x - formW + 1) / partitionSize);
			var minY = Math.floor(Math.max(0, y - formH + 1) / partitionSize);
			var maxX = Math.floor((x + w - 1) / partitionSize);
			var maxY = Math.floor((y + h - 1) / partitionSize);

			for (var px = minX; px <= maxX; px++)
			{
				for (var py = minY; py <= maxY; py++)
				{
					partitions[px][py].push(data);
				}
			}
		};

		var open = [[formX, formY, 1, 1, 0]];
		expand(formX, formY, -1, -1, 0);
		expand(formX, formY, -1, 0, 0);
		expand(formX, formY, 0, -1, 0);

		var best = null;
		var lowestCost = 0;

		for (var index = 0; index < open.length; index++)
		{
			var current = open[index];
			var depth = current[4];
			var x = current[0];
			var y = current[1]
			var px = Math.floor(x / partitionSize);
			var py = Math.floor(y / partitionSize);
			var actors = partitions[px][py];

			// calculate cost
			var cost = eval(current, x, y, depth, actors);

			if (best === null || cost < lowestCost)
			{
				best = current;
				lowestCost = cost;
			}

			var dx = current[2];
			var dy = current[3];

			expand(x, y, dx, dy, depth);

			if (dx !== 0 && dy !== 0)
			{
				expand(x, y, dx, 0, depth);
				expand(x, y, 0, dy, depth);
			}
		}

		return [best[0], best[1]];

		function expand(x, y, dx, dy, depth)
		{
			x += dx;
			y += dy;

			if (dx !== 0 && (x < left || x + formW > right))
				return;

			if (dy !== 0 && (y < top || y + formH > bottom))
				return;

			depth++;

			open.push([x, y, dx, dy, depth]);
		}

		function eval(node, x, y, depth, actors)
		{
			var actorCost = 10;
			var distCost = 1;

			var cost = 0;
			var halfW = formW / 2;
			var halfH = formH / 2;
			var cx = x + halfW;
			var cy = y + halfH;
			var ex = x + formW;
			var ey = y + formH;

			for (var i = actors.length - 1; i >= 0; i--) 
			{
				var data = actors[i];
				var actorX = data.loc[0];
				var actorY = data.loc[1];
				var actorW = data.object.width;
				var actorH = data.object.height;
				var ix = Math.max(x, actorX);
				var iy = Math.max(y, actorY);
				var iw = Math.min(ex, actorX + actorW) - ix;
				var ih = Math.min(ey, actorY + actorH) - iy;

				if (iw > 0 && ih > 0)
				{
					var proximity = Math.max(1 - Math.abs(ix + iw/2 - cx) / halfW, 1 - Math.abs(iy + ih/2 - cy) / halfH);
					if (proximity > 0)
					{
						cost += actorCost * proximity * iw * ih;
					}					
				}
			};

			cost += Math.max(Math.abs(x - formX), Math.abs(y - formY));
			return cost;
		};
	}
});

var Grid = Class(
{
	constructor : function()
	{
		this.objects = [];
		this.boundsDirty = false;
		this.bx = 0;
		this.by = 0;
		this.ex = 0;
		this.ey = 0;
	},

	left : function()
	{
		if (this.boundsDirty) this.calcBounds();
		return this.bx;
	},

	top : function()
	{
		if (this.boundsDirty) this.calcBounds();
		return this.by;
	},

	width : function()
	{
		if (this.boundsDirty) this.calcBounds();
		return this.ex - this.bx;
	},

	height : function()
	{
		if (this.boundsDirty) this.calcBounds();
		return this.ey - this.by;
	},

	place : function(object, x, y)
	{
		var objects = this.objects;
		for (var i = objects.length - 1; i >= 0; i--) 
		{
			if (objects[i].object === object)
				throw ("the object is already in the grid!");
		};

		var loc;
		var ix;
		var iy;

		if (arguments.length >= 3)
		{
			ix = Math.round(x);
			iy = Math.round(y);
			loc = [ix, iy, x - ix, y - iy];
		}
		else
		{
			loc = x;
			ix = loc[0];
			iy = loc[1];
		}
		

		if (!this.boundsDirty)
		{
			if (this.objects.length === 0)
			{
				this.bx = ix;
				this.by = iy;
				this.ex = ix + object.width;
				this.ey = iy + object.height;
			}
			else
			{
				this.bx = Math.min(this.bx, ix);
				this.by = Math.min(this.by, iy);
				this.ex = Math.max(this.ex, ix + object.width);
				this.ey = Math.max(this.ey, iy + object.height);
			}
		}

		var data = { object: object, loc: loc };
		this.objects.push(data);
		return data;
	},

	remove : function(object)
	{
		var objects = this.objects;
		for (var i = objects.length - 1; i >= 0; i--) 
		{
			if (objects[i].object === object)
			{
				objects.splice(i, 1);
				this.boundsDirty = true;
				return;
			}
		};

		throw ("the obeject is not in the grid!");
	},

	removeList : function(objects)
	{
		for (var i = objects.length - 1; i >= 0; i--) 
		{
			this.remove(objects[i]);
		};
	},

	clear : function()
	{
		this.objects.length = 0;
		this.boundsDirty = false;
		this.bx = 0;
		this.by = 0;
		this.ex = 0;
		this.ey = 0;
	},

	calcBounds : function()
	{
		var objects = this.objects;
		if (objects.length === 0)
		{
			this.clear();
			return;
		}

		this.bx = objects[0].loc[0];
		this.by = objects[0].loc[1];
		this.ex = this.bx + objects[0].object.width;
		this.ey = this.by + objects[0].object.height;

		for (var i = objects.length - 1; i >= 0; i--) 
		{
			var data = objects[i];
			var ix = data.loc[0];
			var iy = data.loc[1];
			var object = data.object;
			this.bx = Math.min(this.bx, ix);
			this.by = Math.min(this.by, iy);
			this.ex = Math.max(this.ex, ix + object.width);
			this.ey = Math.max(this.ey, iy + object.height);
		};

		this.boundsDirty = false;
	}
});

