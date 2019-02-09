var Figure = Class(
{
	constructor : function(table, entity, x, y)
	{
		this._table = table;
		this.entity = entity;
		this._def = entity.definition;
		this._id = UniqueId.New("figure_" + this._def.name);
		this._speechBubble = null;


		var sprite = Sprites[this._def.sprite];
		var spriteId = this._id + "_sprite";

		this.node = this._table.figures.addGroup(this._id, { width : "auto", height : "auto"});
		this.node.addSprite(spriteId, {animation: sprite, width: sprite.delta, height: sprite.distance});
		$("#" + spriteId).css("position", "");

		this.node.click(this._onClick.bind(this));

		this.w = this.node.width();
		this.h = this.node.height();

		this.moveTo(x, y);
	},


	text : function(paragraphs, finished)
	{
		if (this._speechBubble === null)
			this._speechBubble = new SpeechBubble(this, this._table);

		this._speechBubble.show(paragraphs, finished);
	},

	rect : function()
	{
		var xy = this.node.xy();
		return { x : xy.x, y : xy.y, w : this.w, h : this.h };
	},

	moveTo : function(x, y)
	{
		var left = x - (this.w / 2);
		var top = y - this.h;
		this.node.xy(left, top);
	},

	_onClick : function(e)
	{
		Events.trigger(this, 'click');
	}
});

