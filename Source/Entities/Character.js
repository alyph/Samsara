'use strict';

class Character extends Entity
{
	constructor()
	{
		super();

		this.displayName ="nameless";
		this.sprite = null;
		this.size = "normal";
		this.party = null;
		this.pov = null;
	}

	getTitle()
	{
		return this.displayName;
	}

	getImage()
	{
		return Sprites[this.sprite];
	}

	getSize()
	{
		return this.size;
	} 
}