'use strict';

/* globals Archive Area ActionTarget Encounter */
/*exported Player*/
class Player extends Entity
{
	constructor()
	{
		super();

		this.deck = new Area();
		this.hand = new Area();
		this.partyArea = new Area();
		this.encounterArea = new Area();

		this.playingCard = null;
		this.cardSelectedCallback = null;

		this.encounter = null;

		this.playerView = document.querySelector("player-view");
	}

	start()
	{
		this.playerView.bind("player", this);
		
		let starterDeck = this.rule.generateStarterDeck(this.world);
		for (let card of starterDeck)
		{
			card.placeIn(this.deck);
		}

		let heroes = this.rule.generateCharacters(this.world, 3);
		for (let hero of heroes)
		{
			hero.placeIn(this.partyArea);
		}

		let initialCards = this.draw(5);
		for (let card of initialCards)
		{
			let hero = MathEx.randomItem(heroes);
			card.attachTo(hero);
		}

		// Create a enoucnter for testing
		this.encounter = this.createEncounter();

		this.refreshView();
	}

	draw(num)
	{
		let drawnCards = [];
		for (let i = 0; i < num && !this.deck.isEmpty(); i++) 
		{
			let card = MathEx.randomItem(this.deck.cards);
			card.placeIn(this.hand);
			drawnCards.push(card);
		}
		return drawnCards;
	}

	selectCard()
	{
		return new Promise((resolve, reject) =>
		{
			if (this.cardSelectedCallback)
				reject(new Error("Already selecting a card."));

			this.cardSelectedCallback = (card) => 
			{
				resolve(card);
			};
		});
	}

	onCardSelected(card)
	{
		if (this.cardSelectedCallback)
		{
			let callback = this.cardSelectedCallback;
			this.cardSelectedCallback = null;
			callback(card);
		}
		// else if (!this.playingCard)
		// {
		// 	this.playingCard = card;
		// 	let player = this;
		// 	this.playCard(card).then(() => { player.playingCard = null; this.refreshView(); });
		// 	this.refreshView();
		// }
	}

	get allCards()
	{
		let allCards = [
			...this.hand.cards, 
			...this.partyArea.cards, 
			...this.encounterArea.cards];

		for (let hero of this.partyArea.cards)
		{
			allCards = allCards.concat(hero.state.attachments);
		}

		return allCards;
	}

	refreshView()
	{
		this.playerView.refresh();
	}

	async playTurn()
	{
		while (true)
		{
			let card = await this.selectCard();

			// TODO: can check if the card is even playable (maybe filter out enemy cards?)
			// Although this should still work, since playCard will just return false.
			if (card)
			{
				if (await this.playCard(card))
				{
					// one action per turn
					break;
				}
				// If not played back to choose next card.
			}
		}

		this.playingCard = null;
		this.refreshView();
	}

	async playCard(card) 
	{
		// TODO: perhaps only update this when we determined the card is playable (i.e. with valid actions)
		this.playingCard = card;
		this.refreshView();

		// Pick valid action
		let action = await this.pickAction(card);
		if (!action)
			return false;

		// Pick target
		let target = await this.pickTarget(card, action);
		if (target === Player.actionCanceled)
			return false;

		// Execute the action
		return await action.execute(target);
	}

	async pickAction(card)
	{
		// Select the first usable action for now
		// TODO: present UI to select between usable actions
		for (let action of card.state.actions)
		{
			if (action.isUsable(card))
			{
				return action;
			}
		}
		return null;
	}

	async pickTarget(card, action)
	{
		if (action.definition.target !== ActionTarget.single)
			return null;

		while (true)
		{
			let target = await this.selectCard();
			if (!target || target === card)
				return Player.actionCanceled;

			if (action.isValidTarget(target))
				return target;
		}
	}

	createEncounter()
	{
		let encounter = this.world.spawn(Encounter);
		encounter.setup();
		return encounter;
	}
}

Player.actionCanceled = Symbol("actionCanceled");
