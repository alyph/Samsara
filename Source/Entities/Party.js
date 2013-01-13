var Party = Class(
{
	constructor : function(game)
	{
		this._game = game;
		this.agenda = [];
		this.stories = [];
		this.story = null;

		this.startMainStory();
	},

	drawAgenda : function(count)
	{
		for (var i = 0; i < count; i++)
			this.agenda.push(this._game.siteDeck.draw());

		this.contextUpdated();
	},

	startMainStory : function()
	{
		this.startStory("Story_Main");
	},

	startStory : function(name, params)
	{
		var story = this._newStory(name, params);
		this.stories = [ story ];
		this._onStoryChanged();
		story.begin();
	},

	pushStory : function(name, params)
	{
		var story = this._newStory(name, params);
		if (this.story !== null)
			this.story.pause();
		this.stories.push(story);
		this._onStoryChanged();
		story.begin();
	},

	_newStory : function(name, params)
	{
		var def = Core.getDef(name);
		var pretext = this.story != null ? this.story.context : undefined;
		var story = new Story(this._game, this, def, pretext);
		for (var name in params)
			story.context["_" + name] = params[name];
		return story;
	},

	_onStoryChanged : function()
	{
		var l = this.stories.length;
		this.story = l > 0 ? this.stories[l-1] : null;
		Events.trigger(this, "StoryChanged");
	},

	startActivity : function(def, entity)
	{
		var params = {};
		params[def.entityParamName] = entity;
		this.pushStory(def.story, params);
	},

	contextUpdated : function()
	{
		this.story.contextUpdated();
	}
});