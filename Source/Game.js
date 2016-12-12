'use strict';

class Game extends BaseObject
{
	constructor()
	{
		super();

		this.world = null;
	}

	start(worldName)
	{
		// this.screen = UI.showScreen("gameScreen");

		// // TODO: the image data should be set in the template
		// this.screen.boardBack.setData(Sprites["Back_Theme"]);
		// this.screen.sceneBack.setData(Sprites["loc_primal_forest"]);


		this.world = Archive.get(worldName);
		if (this.world)
		{
			this.world.beginPlay(this);	
		}
		else
		{
			console.error(`world "${worldName}" not found, game failed to start.`);
		}		
	}

	update()
	{
		this.world.update();
	}
}

