

var CardArea = Class(
{
	constructor : function(table)
	{
		this._table = table;
		this.cards = [];
	},

	placeCard : function(card)
	{
		throw ("must be implemented!");
	}
});

var Orientations =
{
	Horizontal : 1,
	Vertical : 2
};

var VerticalAlignment =
{
	Top : 1,
	Center : 2,
	Bottom : 3	
};

var HorizontalAlignment =
{
	Left : 1,
	Center : 2,
	Right : 3	
};

var CardAreaBox = Class(CardArea,
{
	constructor : function(table, x, y, w, h, options)
	{
		CardAreaBox.$super.call(this, table);

		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
		this.r = x + w;
		this.b = y + h;

		options = options || {};
		this._orientation = options.orientation || Orientations.Horizontal;
		this._verticalAlignment = options.verticalAlignment || VerticalAlignment.Center;
		this._horizontalAlignment = options.horizontalAlignment || HorizontalAlignment.Center;
		this._padding = options.padding || 10;
	},

	placeCard : function(card, slot)
	{
		// TODO: card may be put in empty space

		this.cards.push(card);
		this._rearrange();
	},

	replaceCards : function(cards)
	{
		this.cards = cards;
		this._rearrange();
	},

	_rearrange : function()
	{
		var data = this._precalculate();
		var l = this.cards.length;
		for (var i = 0; i < l; i++)
		{
			var card = this.cards[i];
			if (card !== null)
			{
				var loc = this._getCardLoc(i, data);
				card.show(loc.x, loc.y);
			}
		}		
	},

	_precalculate : function()
	{
		var data = {};
		var span = 0;
		var num = this.cards.length;
		data.start = {};
		var sw, sh;

		if (this._orientation == Orientations.Horizontal)
		{
			sw = num * CARD_WIDTH + (num - 1) * this._padding;
			sh = CARD_HEIGHT;

 		}
 		else
 		{
 			sw = CARD_WIDTH;
 			sh = num * CARD_HEIGHT + (num - 1) * this._padding;
 		}

		switch (this._horizontalAlignment)
		{
		case HorizontalAlignment.Left:
			data.start.x = this.x;
			break;
		case HorizontalAlignment.Center:
			data.start.x = this.x + Math.floor((this.w - sw) / 2);
			break;
		case HorizontalAlignment.Right:
			data.start.x = this.r - sw;
			break;
		}

		switch (this._verticalAlignment)
		{
		case VerticalAlignment.Top:
			data.start.y = this.y;
			break;
		case VerticalAlignment.Center:
			data.start.y = this.y + Math.floor((this.h - sh) / 2);
			break;
		case VerticalAlignment.Bottom:
			data.start.y = this.sh;
			break;
		}

		return data;
	},

	_getCardLoc : function(idx, data)
	{
		if (this._orientation == Orientations.Horizontal)
		{
			return { x : data.start.x + idx * (CARD_WIDTH + this._padding), y : data.start.y };
		}
		else
		{
			return { x : data.start.x, y : data.start.y + idx * (CARD_HEIGHT + this._padding) };
 		}
	}
});


