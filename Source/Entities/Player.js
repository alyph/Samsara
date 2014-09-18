var Player = Class(
{
	constructor : function(game)
	{
		this.game = game;
		this.party = game.world.spawn("PlayerParty", { $base: Party });//new Party(game);
		this.isPlaying = false;
		this.remainingActions = [];
		this.queuedActions = [];

		//game.screen.actions.on("click", this.onAction, this);
	},

	play : function(finished)
	{
		var screen = this.game.screen;
		var scene = this.party.scene;
		screen.title.setData("~ " + scene.environment.desc + " ~");
		screen.features.setData("Open Ground • River • Bushes • Trees • Boulders • Dead Bodies");
		screen.stage.setData(scene.stage);
		screen.prompts.setData("You are approached by a goblin warband led by a warg rider. They are soon rushing towards you. The balky barbarian Xaross now has a chance to act...");

		this.remainingActions = this.party.plannedActions; 
		//screen.actions.setData(this.remainingActions);
		this.playFinished = finished;

		this.queuedActions.length = 0;
		this.isPlaying = true;
	},

	onAction : function(e)
	{
		if (!this.isPlaying)
			return;

		var item = e.targetItem;
		if (!item)
			return;

		this.queueAction(item.data);
	},

	queueAction : function(action)
	{
		var index = this.remainingActions.indexOf(action);
		if (index < 0)
			throw ("queued action not available!");

		this.remainingActions.splice(index, 1);
		this.queuedActions.push(action);

		//this.game.screen.actions.setData(this.remainingActions);

		if (this.remainingActions.length === 0)
		{
			this.isPlaying = false;
			this.assignActions();
			this.playFinished();
		}
	},

	assignActions : function()
	{
		this.party.actions.length = 0;

		for (var i = 0; i < this.queuedActions.length; i++) 
		{
			this.party.actions.push(this.queuedActions[i]);
		};
	},

	preStep : function()
	{
		return this.party.preStep();
	},

	step : function()
	{
		return this.party.step();
	} 
});