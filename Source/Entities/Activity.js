var Activity = Class(
{
	constructor : function(game, party)
	{
		this._game = game;
		this._party = party;

		this.context = {};
		this.context.party = party;
		this.context.activity = this;

		this.changeScene(new Scene(this.context));
	},

	changeScene : function(scene)
	{
		this.scene = scene;
		Events.trigger(this, "SceneChanged");
	},

	contextUpdated : function()
	{
		this.scene.refresh();
	}
});

var Scene = Class(
{
	constructor : function(context)
	{
		this._context = context;
		this.contextualEntities = [];
		this.sceneCard = null;
		this.entityCards = [];
	},

	refresh : function()
	{
		this._populateEntityCards();
		Events.trigger(this, "SceneUpdated");
	},

	_populateEntityCards : function()
	{
		this.entityCards.length = 0;
		for (var i = 0; i < this.contextualEntities.length; i++)
		{
			var paths = this.contextualEntities[i].split(".");
			var entities = this._context;
			for (var p = 0; p < paths.length; p++)
			{
				entities = entities[paths[p]];
				if (entities === undefined)
					throw ("Incorrect Contextual Entity Path:" + paths + ", cannot find" + paths[p]);
			}
			this.entityCards = this.entityCards.concat(entities);
		}
	}
});
