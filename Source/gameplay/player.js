'use strict';

/* globals Archive Area ActionTarget */
/*exported Player*/
class Player extends Entity
{
	constructor()
	{
		super();

		this.deck = new Area();
		this.hand = new Area();

		this.playingCard = null;
		this.cardSelectedCallback = null;

		this.playerView = document.querySelector("player-view");
	}

	start()
	{
		this.playerView.bind("player", this);
		
		let hero = this.world.spawn("hero.paladin");
		hero.placeIn(this.world.field.partyArea);
		//hero.place(this.world, FieldArea.Party);

		let sword = this.world.spawn("equipment.longsword");
		sword.placeIn(this.deck);

		this.draw(1);

		this.refreshView();
	}

	draw(num)
	{
		for (let i = 0; i < num && !this.deck.isEmpty(); i++) 
		{
			let card = MathEx.randomElementOfArray(this.deck.cards);
			card.placeIn(this.hand);
		}
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
}

Player.actionCanceled = Symbol("actionCanceled");
