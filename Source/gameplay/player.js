'use strict';

/* globals Archive Area ActionTarget Encounter EventDispatcher */
/*exported Player*/
class Player extends Entity
{
	constructor()
	{
		super();

		this.events = new EventDispatcher();

		this.deck = new Area();
		this.hand = new Area();
		this.partyArea = new Area();
		this.encounterArea = new Area();
		this.activeActionArea = new Area();

		this.playingCard = null;
		this.cardSelectedCallback = null;

		this.encounter = null;
	}

	start()
	{
		this.playerView = document.querySelector("player-view");
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
		this.encounter = this.rule.generateEncounter(this.world);
		this.encounterArea.placeCards(this.encounter.entities);

		this.refreshView();
	}

	get allCards()
	{
		let allCards = [
			...this.hand.cards, 
			...this.partyArea.cards, 
			...this.encounterArea.cards,
			...this.activeActionArea.cards];

		for (let hero of this.partyArea.cards)
		{
			allCards = allCards.concat(hero.state.attachments);
		}

		return allCards;
	}

	get activeActionAddonArea()
	{
		if (this.activeActionArea.length > 0)
		{
			return this.activeActionArea[0].addon;
		}

		return null;
	}

	get hasActiveAction()
	{
		return this.activeActionArea.cards.length > 0;
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

	async selectCard()
	{
		let e = await this.events.waitFor(CardSelected);
		return e.card;

		// return new Promise((resolve, reject) =>
		// {
		// 	if (this.cardSelectedCallback)
		// 		reject(new Error("Already selecting a card."));

		// 	this.cardSelectedCallback = (card) => 
		// 	{
		// 		resolve(card);
		// 	};
		// });
	}

	// onCardSelected(card)
	// {
	// 	if (this.cardSelectedCallback)
	// 	{
	// 		let callback = this.cardSelectedCallback;
	// 		this.cardSelectedCallback = null;
	// 		callback(card);
	// 	}
	// 	// else if (!this.playingCard)
	// 	// {
	// 	// 	this.playingCard = card;
	// 	// 	let player = this;
	// 	// 	this.playCard(card).then(() => { player.playingCard = null; this.refreshView(); });
	// 	// 	this.refreshView();
	// 	// }
	// }


	refreshView()
	{
		this.playerView.refresh();
	}

	async playTurn()
	{
		let canContinue = true;
		while (canContinue)
		{
			canContinue = await this.performAction();
		}

		// while (true)
		// {
		// 	let card = await this.selectCard();

		// 	// TODO: can check if the card is even playable (maybe filter out enemy cards?)
		// 	// Although this should still work, since playCard will just return false.
		// 	if (card)
		// 	{
		// 		if (await this.playCard(card))
		// 		{
		// 			// one action per turn
		// 			break;
		// 		}
		// 		// If not played back to choose next card.
		// 	}
		// }

		// this.playingCard = null;
		// this.refreshView();
	}

	async performAction()
	{
		let canContinue = true;
		let hero = null;
		while (true)
		{
			let card = await this.selectCard();

			// TODO: check if the hero is recovering
			if (card.getArea() === this.partyArea)
			{
				hero = card;
				break;
			}
		}

		// Activate the hero (so he becomes the instigator of the action)
		this.activateCard(hero);
		this.refreshView();

		// Now select an common action target 
		// (the target that has at least one common action that can be perfomed by the hero)
		let { target, actions } = await this.selectCommonActionTarget(hero);

		// If target is null, that means the action is canceled, return true to try next action
		if (target)
		{
			this.targetCard(target);

			// TODO: if more than one common action, select one.

			let action = actions[0];
			let actionCard = action.spawnActionCard();
			actionCard.placeIn(this.activeActionArea); // more for the UI.

			this.refreshView();

			// Prepare for action (if return false, means canceled)
			let isConfirmed = await this.prepareForAction(action);

			if (isConfirmed)
			{
				let shouldAdvanceTime = await action.execute();
				canContinue = !shouldAdvanceTime;
			}

			actionCard.destroy();

			// Return all the used cards to previous area.
			// Note if some cards are discarded in the process of the action,
			// returnToArea() will basically do nothing, since the card is
			// no longer in a temp area.
			for (let addonCard of action.addon.cards)
			{
				addonCard.returnToArea();
			}

			this.untargetCard(target);
		}
		
		// Deactivate hero
		this.deactivateCard(hero);
		this.refreshView();

		return canContinue;
	}

	async selectCommonActionTarget(instigator)
	{
		let target = null;
		let actions = [];

		while (true)
		{
			let card = await this.selectCard();
			if (card === instigator)
			{
				break;
			}
			else
			{
				actions = this.rule.populateDefaultActions(this.world, instigator, card);
				if (actions && actions.length > 0)
				{
					target = card;
					break;
				}
			}
		}

		return { target, actions };
	}

	async prepareForAction(action)
	{
		while (true)
		{
			let e = await this.events.waitFor(CardSelected, ActionConfirmed, ActionCanceled);
			if (e.constructor === CardSelected)
			{
				let card = e.card;
				if (card.isUsableOnAction(action))
				{
					card.placeIn(action.addon);
				}
			}
			else if (e.constructor === ActionConfirmed)
			{
				return true;
			}
			else if (e.constructor === ActionCanceled)
			{
				return false;
			}
		}
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

	activateCard(card)
	{
		card.state.activated = true;
	}

	deactivateCard(card)
	{
		card.state.activated = false;
	}

	targetCard(card)
	{
		card.state.targeted = true;
	}

	untargetCard(card)
	{
		card.state.targeted = false;
	}
}

Player.actionCanceled = Symbol("actionCanceled");

class CardSelected
{
	constructor(card)
	{
		this.card = card;
	}
}

class ActionConfirmed
{
	constructor() {}
}

class ActionCanceled
{
	constructor() {}
}
