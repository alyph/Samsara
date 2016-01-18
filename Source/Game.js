'use strict';

class Game extends BaseObject
{
	constructor()
	{
		super();

		this.world = null;
	}

	start()
	{
		// this.screen = UI.showScreen("gameScreen");

		// // TODO: the image data should be set in the template
		// this.screen.boardBack.setData(Sprites["Back_Theme"]);
		// this.screen.sceneBack.setData(Sprites["loc_primal_forest"]);


		this.world = Archive.get(Global.WorldName);
		this.world.beginPlay(this);
	}

	update()
	{
		this.world.update();
	}
}

