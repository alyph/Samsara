'use strict';

/* globals Area */

/*exported Field*/
class Field extends Entity
{
	constructor()
	{
		super();

		this.partyArea = new Area();
		this.encounterArea = new Area();
	}
}



