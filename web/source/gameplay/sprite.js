'use strict';

/* exported Sprite */
class Sprite
{
	constructor()
	{
		this.sheet = null;
		this.x = 0;
		this.y = 0;
	}
}

/* exported SpriteSheet */
class SpriteSheet
{
	constructor()
	{
		this.url = "";
		this.DOM = null;
		this.w = 0;
		this.h = 0;
		this.tilesx = 1;
		this.tilesy = 1;
	}

	[Archive.Symbol.loaded](record)
	{
		let sheet = this;

		return new Promise((resolve, reject) => 
		{			
			sheet.DOM = new Image();
			//img.DOM.id = name;
			sheet.DOM.addEventListener("load", imageLoaded);
			sheet.DOM.src = sheet.url;

			function imageLoaded(event)
			{
				if (sheet.DOM.naturalWidth && sheet.DOM.naturalHeight)
				{
					sheet.tilesx = sheet.w > 0 ?
						Math.round(sheet.DOM.naturalWidth / sheet.w) : 1;

					sheet.tilesy = sheet.h > 0 ?
						Math.round(sheet.DOM.naturalHeight / sheet.h) : 1;

					resolve(sheet);
				}
				else
				{
					reject(new Error(`Failed to load image: ${sheet.url}`));
				}
			}
		});		
	}
}


