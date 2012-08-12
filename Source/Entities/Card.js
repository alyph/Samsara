var Card = Class(
{
	constructor : function(game, info, id)
	{
		this._game = game;
		this._table = game.table;
		this._info = info;
		this._def = info.definition;
		this._width = CARD_WIDTH;
		this._height = CARD_HEIGHT;
		this.Node = this._table.Node.addGroup("card_"+id, {width : this._width, height : this._height});
		var back = $("<div class='cardBack'></div>").appendTo(this.Node);
		var label = $("<div class='cardTitle'></div>").html("Ghost").appendTo(back);
		var desc = $("<div class='cardDesc'></div>").html("Attack + 1").appendTo(back);
	},

	move : function(x, y)
	{
		this.Node.xy(x, y, true);
	}
});