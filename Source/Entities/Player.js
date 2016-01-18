'use strict';

/*exported Player*/
class Player extends Entity
{
	constructor()
	{
		//Player.$super.call(this);
		super();

		//this.screen = null;
		this.pov = null;
		this.actions = null;
		this.pendingActions = null;
		this.backdrop = null;
		this.events = new EventDispatcher();
		this.waiting = false;

		$("player-view")[0].bind(this);
		//this.mediator = new PlayerMediator(this);
		// this.game = game;
		// this.party = game.world.spawn("PlayerParty", { $base: Party });//new Party(game);
		// this.party.player = this;
		// this.isPlaying = false;
		// this.remainingActions = [];
		// this.queuedActions = [];

		//game.screen.actions.on("click", this.onAction, this);
	}

	// enterWorld : function(world)
	// {
	// 	Player.$superp.enterWorld.call(this, world);
		
	// 	this.screen = world.game.screen;
	// },

	// beginPlay : function(game)
	// {
	// 	//this.screen = game.screen;
		
	// },

	chooseAction(pov, actions)
	{
		if (this.pov !== null)
			throw ("Another pov choosing action.");

		this.pov = pov;
		this.pendingActions = actions;

		var newScene = pov.scene;
		if (this.actions !== null)
		{
			for (var i = 0; i < this.actions.length; i++) 
			{
				if (this.actions[i].target === newScene)
				{
					this.beginTransitionFromAction(this.actions[i], newScene);
					return;
				}
			}
		}

		this.finishTransition();

		//this.actions = actions;

		// this.mediator.beginChooseAction();

		

		// TODO: UI
		//this.screen.scene.setData(pov.scene);
	}

	beginTransitionFromAction(action, scene)
	{
		var panel = new Panel();
		panel.portrait = scene.portrait;
		var event = new PanelAddedEvent(panel);
		event.fromAction = action;
		this.events.dispatch(event);
	}

	finishTransition()
	{
		this.backdrop = this.pov.scene.Backdrop();
		this.actions = this.pendingActions;
		this.waiting = true;
	}

	finishChooseAction(action)
	{
		if (this.waiting)
		{
			this.waiting = false;
			var pov = this.pov;		
			this.pov = null;
			pov.actionChosen(action);
		}
	}

	// play : function(finished)
	// {
	// 	var screen = this.game.screen;
	// 	var scene = this.party.scene;
	// 	screen.title.setData("~ " + scene.environment.desc + " ~");
	// 	screen.features.setData("Open Ground • River • Bushes • Trees • Boulders • Dead Bodies");
	// 	screen.stage.setData(scene.stage);
	// 	screen.prompts.setData("You are approached by a goblin warband led by a warg rider. They are soon rushing towards you. The balky barbarian Xaross now has a chance to act...");

	// 	this.remainingActions = this.party.plannedActions; 
	// 	//screen.actions.setData(this.remainingActions);
	// 	this.playFinished = finished;

	// 	this.queuedActions.length = 0;
	// 	this.isPlaying = true;
	// },

	// showScene : function(scene)
	// {
	// 	var screen = this.game.screen;
	// 	var scene = this.party.scene;
	// 	screen.title.setData("~ " + scene.environment.desc + " ~");
	// 	screen.features.setData("Open Ground • River • Bushes • Trees • Boulders • Dead Bodies");
	// 	screen.stage.setData(scene.stage);
	// 	screen.prompts.setData("You are approached by a goblin warband led by a warg rider. They are soon rushing towards you. The balky barbarian Xaross now has a chance to act...");
	// },

	// chooseAction : function(actionInfos, finished)
	// {
	// 	throw ("not implemented!");
	// },

	// onAction : function(e)
	// {
	// 	if (!this.isPlaying)
	// 		return;

	// 	var item = e.targetItem;
	// 	if (!item)
	// 		return;

	// 	this.queueAction(item.data);
	// },

	// queueAction : function(action)
	// {
	// 	var index = this.remainingActions.indexOf(action);
	// 	if (index < 0)
	// 		throw ("queued action not available!");

	// 	this.remainingActions.splice(index, 1);
	// 	this.queuedActions.push(action);

	// 	//this.game.screen.actions.setData(this.remainingActions);

	// 	if (this.remainingActions.length === 0)
	// 	{
	// 		this.isPlaying = false;
	// 		this.assignActions();
	// 		this.playFinished();
	// 	}
	// },

	// assignActions : function()
	// {
	// 	this.party.actions.length = 0;

	// 	for (var i = 0; i < this.queuedActions.length; i++) 
	// 	{
	// 		this.party.actions.push(this.queuedActions[i]);
	// 	};
	// },

	// preStep : function()
	// {
	// 	return this.party.preStep();
	// },

	// step : function()
	// {
	// 	return this.party.step();
	// } 
}

class PanelAddedEvent
{
	constructor(panel)
	{
		this.panel = panel;
		this.fromAction = null;
	}
}

class Panel
{
	constructor()
	{
		this.portrait = null;
	}
}

// class PlayerMediator
// {
// 	constructor(player)
// 	{
// 		this.player = player;
// 		this.backdrop = null;
// 		this.actions = null;
// 		this.events = new EventDispatcher();

// 		$("player-view")[0].bind(this);
// 	}

// 	beginChooseAction()
// 	{
// 		var newScene = this.player.pov.scene;

// 		if (this.actions !== null)
// 		{
// 			for (var i = 0; i < this.actions.length; i++) 
// 			{
// 				if (this.actions[i].target === newScene)
// 				{
// 					this.beginTransitionFromAction(this.actions[i], newScene);
// 					return;
// 				}
// 			}
// 		}

// 		this.finishTransition();
// 	}

// 	finishChooseAction(action)
// 	{
// 		this.player.finishChooseAction(action);
// 	}

// 	beginTransitionFromAction(action, scene)
// 	{
// 		var panel = new Panel();
// 		panel.portrait = scene.portrait;
// 		var event = new PanelAddedEvent(panel);
// 		event.fromAction = action;
// 		this.events.dispatch(event);
// 	}

// 	finishTransition()
// 	{
// 		this.backdrop = this.player.pov.scene.Backdrop();
// 		this.actions = this.player.actions;
// 	}
// }