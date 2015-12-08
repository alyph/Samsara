var Game = Class(BaseObject,
{
	constructor : function()
	{
		Game.$super.call(this);
	},

	start : function()
	{
		// this.screen = UI.showScreen("gameScreen");

		// // TODO: the image data should be set in the template
		// this.screen.boardBack.setData(Sprites["Back_Theme"]);
		// this.screen.sceneBack.setData(Sprites["loc_primal_forest"]);


		this.world = Archive.get(Global.WorldName);
		this.world.beginPlay(this);
	},

	update : function()
	{
		this.world.update();
	}
});

