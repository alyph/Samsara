var Player = Class(
{
	constructor : function(game)
	{
		this._game = game;
		this.Deck = new PlayerDeck(CommonDecks.PlayerStartingDeck);
		this.HandSize = 5;
	}
});