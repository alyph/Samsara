var Player = Class(
{
	constructor : function(game)
	{
		this._game = game;
		this.deck = new PlayerDeck(game);
		this.handSize = 5;
	}
});