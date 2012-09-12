var Player = Class(
{
	constructor : function(game)
	{
		this._game = game;
		this.deck = new PlayerDeck(CommonDecks.PlayerStartingDeck);
		this.heroDeck = new PlayerDeck(CommonDecks.PlayerStartingHeroes);
		this.handSize = 5;
	}
});