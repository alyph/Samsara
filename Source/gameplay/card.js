/* globals World Trait ActionInstance*/

/* exported Card */
class Card
{
	constructor()
	{
		this.displayName = "NO NAME";
		this.smallPortrait = null;
		this.definition = new Trait();
		this.state = null;
	}

	[World.Symbol.start](world)
	{
		let state = new CardState();
		state.world = world;

		// TODO: actual populating/caching code
		//state.actions.push(...this.definition.actions);
		for (let action of this.definition.actions)
		{
			state.actions.push(new ActionInstance(this, action));
		}

		this.state = state;
	}

	placeIn(area)
	{
		if (!this.state)
			throw ("Cannot place non-intanced cards.");

		if (area !== this.state.area)
		{
			let prevArea = this.state.area;
			if (prevArea)
			{
				let idx = prevArea.cards.indexOf(this);
				if (idx >= 0)
					prevArea.cards.splice(idx, 1);
			}

			this.state.area = area;
			if (area)
			{
				area.cards.push(this);
			}
		}
	}

	isOnField()
	{
		let state = this.state;
		let field = state.world.field;
		return state.area === field.partyArea ||
			state.area === field.encounterArea;
	}

	attachTo(card)
	{
		if (!this.state)
			throw ("Cannot attach non-intanced cards.");

		if (!card.state)
			throw ("Cannot attach to non-intanced cards.");

		this.placeIn(null);

		let prevHost = this.state.host;
		if (prevHost)
		{
			let idx = prevHost.state.attachments.indexOf(this);
			if (idx >= 0)
				prevHost.state.attachments.splice(idx, 1);
		}

		this.state.host = card;
		if (card)
		{
			card.state.attachments.push(this);
		}
	}
}

class CardState
{
	constructor()
	{
		this.world = null;
		this.area = null;
		this.actions = [];
		this.host = null;
		this.attachments = [];
	}
}

// class CardTemplate
// {
// 	constructor()
// 	{
// 		// any hand cards
// 		this.playEffects = [];

// 		// any attachment or minion cards
// 		this.triggeredEffects = [];
// 		this.modifiers = [];

// 		// any attachment cards
// 		this.activatableEffects = [];

// 		// actor cards (heroes, minions)
// 		this.traits = [];


// 	}
// }

