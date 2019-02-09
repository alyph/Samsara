'use strict';

/* exported Game */
class Game
{
	constructor()
	{
		this.world = null;
	}

	start(worldName)
	{
		// this.screen = UI.showScreen("gameScreen");

		// // TODO: the image data should be set in the template
		// this.screen.boardBack.setData(Sprites["Back_Theme"]);
		// this.screen.sceneBack.setData(Sprites["loc_primal_forest"]);


		this.world = Archive.getInst(worldName);
		if (this.world)
		{
			this.world.start(this);
			this.run();
		}
		else
		{
			console.error(`world "${worldName}" not found, game failed to start.`);
		}
	}

	async run()
	{
		while (true)
		{
			// 1 Turn

			// environment action (based on action queue) (enemy, encounter etc.)

			// player choose an action
			await this.world.player.playTurn();

			// post turn action (usually hidden, for drawing environment or world update in the future)
		}
	}

	// update()
	// {
	// 	this.world.update();
	// }
}

