/* globals Vector2 */

class CardInfo
{
	constructor()
	{
		this.cardProxy = null;
		this.center = new Vector2();
		this.attachments = [];
	}
}

class AreaProxyBase extends UI.Proxy
{
	constructor()
	{
		super();

		this.expandUpwards = false;
	}
}

/* exported AreaProxy */
class AreaProxy extends AreaProxyBase
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
		this.horizontalPadding = 200;
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

/* exported PartyAreaProxy */
class PartyAreaProxy extends AreaProxyBase
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
		this.characterPadding = 200;
		this.attachmentPadding = 160;
		this.paddingBetweenCharacters = 64;

		this.cardInfos = [];

		this.expandUpwards = true; // TODO: this should be set in the view html.
	}

	onInit()
	{
		[this.left, this.top] = this.component.getHostOffsetToRoot();
		[this.width, this.height] = this.component.getHostSize();
	}

	onUpdate()
	{
		let cards = this.area.cards;
		let numCards = cards.length;

		let needUpdateLayout = false;
		if (this.cardInfos.length !== numCards)
		{
			needUpdateLayout = true;
		}
		else
		{
			for (let i = 0; i < numCards; i++)
			{
				if (cards[i].state.attachments.length !== this.cardInfos[i].attachments.length)
				{
					needUpdateLayout = true;
					break;
				}
			}
		}

		if (needUpdateLayout)
		{
			if (numCards > 0)
			{
				let numChars = numCards;
				let numAttachments = 0;

				for (let i = 0; i < numChars; i++)
				{
					numAttachments += cards[i].state.attachments.length;
				}

				let minWidthPerChar = Math.max(this.characterPadding, this.attachmentPadding);
				let paddingBetween = (numChars === 1 ? 0:
					((this.width - minWidthPerChar * numChars) / (numChars - 1)));

				paddingBetween = MathEx.clamp(paddingBetween, 
					-Math.floor(minWidthPerChar/2), this.paddingBetweenCharacters);

				
				let minWidth = minWidthPerChar * numChars + paddingBetween * (numChars - 1);
				let spaceRemaining = Math.max((this.width - minWidth), 0);
				let spaceRequired = this.attachmentPadding * numAttachments;

				let attachmentPadding = this.attachmentPadding;
				if (spaceRemaining < spaceRequired)
				{
					attachmentPadding = Math.floor(spaceRemaining / numAttachments);
				}

				let totalWidth = minWidth + attachmentPadding * numAttachments;
				let left = Math.floor((this.width - totalWidth) / 2) + this.left;
				let y = Math.floor(this.height / 2) + this.top;
				let halfCharPadding = Math.floor(this.characterPadding / 2);
				let halfAttachmentPadding = Math.floor(attachmentPadding / 2);

				this.cardInfos.length = numChars;
				for (let i = 0; i < numChars; i++)
				{
					let card = cards[i];
					let charInfo = this.cardInfos[i];
					if (!charInfo)
						charInfo = this.cardInfos[i] = new CardInfo();

					charInfo.center.x = left + halfCharPadding;
					charInfo.center.y = y;

					left += (minWidthPerChar - halfAttachmentPadding);

					let numAttachmentsThisChar = card.state.attachments.length;
					charInfo.attachments.length = numAttachmentsThisChar;
					for (let ai = 0; ai < numAttachmentsThisChar; ai++)					
					{
						let attachInfo = charInfo.attachments[ai];
						if (!attachInfo)
							attachInfo = charInfo.attachments[ai] = new CardInfo();

						left += attachmentPadding;
						attachInfo.center.x = left;
						attachInfo.center.y = y;
					}

					left += (halfAttachmentPadding + paddingBetween);
				}

			}
			else
			{
				this.cardInfos.length = 0;
			}
		}

		for (let i = 0; i < numCards; i++)
		{
			let card = cards[i];
			let cardInfo = this.cardInfos[i];
			syncCardAndCardInfo(cardInfo, card);

			for (let ai = 0; ai < cardInfo.attachments.length; ai++)
			{
				syncCardAndCardInfo(cardInfo.attachments[ai], card.state.attachments[ai]);
			}
		}
	}
}

function syncCardAndCardInfo(cardInfo, card)
{
	if (!cardInfo)
	{
		throw ("Missing card info.");
	}

	if (!cardInfo.cardProxy ||
		cardInfo.cardProxy.card !== card)
	{
		// will get null if card is null.
		cardInfo.cardProxy = UI.findOrCreateProxy(CardProxy, card);
	}

	let cardProxy = cardInfo.cardProxy;

	// TODO: ignore cards that are temporarily away. 
	// or the center would not be the final center.
	if (cardProxy && !cardProxy.center.equals(cardInfo.center))
	{
		cardProxy.center.copy(cardInfo.center);
		cardProxy.refreshUI();
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

	get areaProxy()
	{
		let area = this.card.state.area;
		return UI.findProxy(area);
	}

	get expandUpwards()
	{
		let areaProxy = this.areaProxy;
		return areaProxy ? areaProxy.expandUpwards : false;
	}

	get isLiving()
	{
		return this.card.desc.hasAllDescriptors(Archive.getDef("desc.living"));
	}

	get health()
	{
		// TODO: is there a better place to specify the health attribute?
		// considering UI is basically data, should it use the similar syntax
		// like in other data files? like #attr.health
		// maybe instead of having native code, it should use the same scripting language
		// that other gameplay data uses.
		//return this.card.attributeManager.getAttribute(Archive.getDef("attr.health"));
		return this.card.getAttributeValue(Archive.getDef("attr.health"));
	}
}