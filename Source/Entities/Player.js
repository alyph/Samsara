var Player = Class(
{
	constructor : function(game)
	{
		this._game = game;
		this.Deck = new Deck(Decks.PlayerStartingDeck);
		this.HandSize = 5;
	}
});