/* globals Vector2 */

class CardInfo
{
	constructor()
	{
		this.cardProxy = null;
		this.center = new Vector2();
	}
}

/* exported AreaProxy */
class AreaProxy extends UI.Proxy
{
	constructor(area)
	{
		super();
		this.area = area;

		this.left = 0;
		this.top = 0;
		this.width = 0;
		this.height = 0;

		//this.padding = new Vector2();
		this.horizontalPadding = 100;
		this.verticalPadding = 100;
		this.maxRows = 1;

		this.cardInfos = [];
	}

	onInit()
	{
		[this.left, this.top] = this.component.getHostOffsetToRoot();
		[this.width, this.height] = this.component.getHostSize();
	}

	onUpdate()
	{
		let area = this.area;
		let numCards = area.cards.length;
		if (this.cardInfos.length !== numCards)
		{
			this.cardInfos.length = 0;

			if (numCards > 0)
			{
				let maxCardsPerRow = Math.max(1, Math.floor(this.width / this.horizontalPadding));
				let numRows = MathEx.clamp(Math.round(numCards / maxCardsPerRow), 1, this.maxRows);
				let numCardsPerRow = Math.floor(numCards / numRows);
				let numCardsLeftOver = (numCards - numCardsPerRow * numRows);

				let marginY = Math.min(this.height, this.verticalPadding);
				let paddingY = (numRows > 1 ? 
					Math.min(this.verticalPadding, Math.floor((this.height - marginY) / (numRows - 1))) : 0);
				let top = Math.floor((this.height - paddingY * (numRows - 1)) / 2) + this.top;
				
				for (let i = 0; i < numRows; i++)
				{
					let numCardsThisRow = numCardsPerRow;
					if (numCardsLeftOver > 0)
					{
						numCardsThisRow++;
						numCardsLeftOver--;
					}

					let marginX = Math.min(this.width, this.horizontalPadding);
					let paddingX = (numCardsThisRow > 1 ?
						Math.min(this.horizontalPadding, Math.floor((this.width - marginX) / (numCardsThisRow - 1))) : 0);
					let left = Math.floor((this.width - paddingX * (numCardsThisRow - 1)) / 2) + this.left;

					for (let c = 0; c < numCardsThisRow; c++)
					{
						let newCardInfo = new CardInfo();
						newCardInfo.center.x = left + paddingX * c;
						newCardInfo.center.y = top + paddingY * i;
						this.cardInfos.push(newCardInfo);
					}
				}
			}
		}

		for (let i = 0; i < numCards; i++)
		{
			let card = area.cards[i];
			let cardInfo = this.cardInfos[i];
			let cardProxy = UI.findOrCreateProxy(CardProxy, card); // will get null if card is null.
			if (cardInfo.cardProxy !== cardProxy)
			{
				cardInfo.cardProxy = cardProxy;
				if (cardProxy) // TODO: ignore cards that are temporarily away.
				{
					cardProxy.center.copy(cardInfo.center);
					cardProxy.refreshUI();
				}
			}
		}
	}
}

/* exported CardProxy */
class CardProxy extends UI.Proxy
{
	constructor(card)
	{
		super();
		this.card = card;
		this.center = new Vector2(); 
	}
}