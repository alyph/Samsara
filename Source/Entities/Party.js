var Party = Class(
{
	constructor : function(game)
	{
		this._game = game;
		this.agenda = [];

		this.worldActivity = new Activity(game, this);
		this.worldActivity.scene.sceneCard = game.makeCard(new CardInstance(Core.getCards('Story')[0]));
		this.worldActivity.scene.contextualEntities.push("party.agenda");

		this.activityStack = [];
		this.pushActivity(this.worldActivity);
	},

	drawAgenda : function(count)
	{
		for (var i = 0; i < count; i++)
			this.agenda.push(this._game.locationDeck.draw());

		this.contextUpdated();
	},

	pushActivity : function(activity)
	{
		this.activityStack.push(activity);

		// TODO: start activity

		this.onActivityChanged();
	},

	onActivityChanged : function()
	{
		this.currentActivity = this.activityStack[this.activityStack.length - 1];
		Events.trigger(this, "ActivityChanged");
	},

	contextUpdated : function()
	{
		this.currentActivity.contextUpdated();
	}
});