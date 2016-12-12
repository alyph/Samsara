'use strict';

/*exported Player*/
class Player extends Entity
{
	constructor()
	{
		super();

		this.playerView = document.querySelector("player-view");		
	}

	beginPlay()
	{
		this.playerView.bind("player", this);
		
		let hero = Archive.create("heroes.paladin");
		hero.place(this.world, FieldArea.Party);
		this.playerView.refresh();
	}




	// constructor()
	// {
	// 	super();

	// 	this.pov = null;
	// 	this.actions = null;
	// 	this.pendingActions = null;
	// 	this.backdrop = null;
	// 	this.events = new EventDispatcher();
	// 	this.waiting = false;
	// 	this.paragraphs = [];

	// 	this.playerView = document.querySelector("player-view");
	// 	this.playerView.bind("player", this);

	// 	//$("player-view")[0].bind("player", this);
	// }

	// chooseAction(pov, actions)
	// {
	// 	if (this.pov !== null)
	// 		throw ("Another pov choosing action.");

	// 	this.pov = pov;

	// 	this.paragraphs.length = 0;
	// 	this.paragraphs.push(new Narrative(this.pov.scene.description));
	// 	this.paragraphs.push(new ActionChoice(actions));

	// 	this.waiting = true;
	// 	this.playerView.refresh();
	// }

	// beginTransitionFromAction(action, scene)
	// {
	// 	var panel = new Panel();
	// 	panel.portrait = scene.portrait;
	// 	var event = new PanelAddedEvent(panel);
	// 	event.fromAction = action;
	// 	this.events.dispatch(event);
	// }

	// finishTransition()
	// {
	// 	this.backdrop = this.pov.scene.Backdrop();
	// 	this.actions = this.pendingActions;
	// 	this.waiting = true;
	// }

	// finishChooseAction(action)
	// {
	// 	if (this.waiting)
	// 	{
	// 		this.waiting = false;
	// 		var pov = this.pov;		
	// 		this.pov = null;
	// 		pov.actionChosen(action);
	// 	}
	// }
}

// class Paragraph
// {
// 	constructor()
// 	{
// 	}
// }

// class Narrative extends Paragraph
// {
// 	constructor(text)
// 	{
// 		super();
// 		this.text = text;
// 	}
// }

// class ActionChoice extends Paragraph
// {
// 	constructor(actions)
// 	{
// 		super();
// 		this.actions = actions;
// 	}
// }

// class PanelAddedEvent
// {
// 	constructor(panel)
// 	{
// 		this.panel = panel;
// 		this.fromAction = null;
// 	}
// }

// class Panel
// {
// 	constructor()
// 	{
// 		this.portrait = null;
// 	}
// }


