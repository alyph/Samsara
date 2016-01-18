'use strict';

class Party extends Entity
{
	constructor()
	{
		super();

		//this.controller = null;
		this.members = [];
		//this.Locale = null;

		//this.needsUpdate = true;

		//this.player = null;

		// this.scene = null;
		// this.quests = [];
		// this.actions = [];
		// this.plannedActions = [];
		// this.actionPool = [];
		// this.context = {};
		// this.context.party = this;

		// this.questActivities = [];
		// this.localActivities = [];

		// this.scene = null;
	}

	loaded()
	{
		for (var i = this.members.length - 1; i >= 0; i--) 
		{
			this.members[i].party = this;
		}
	}

	addMember(member)
	{
		this.members.push(member);
	}

	// preStep : function()
	// {
	// 	var scene = this.scene;
	// 	return scene === null || scene.preStep();
	// },

	// step : function()
	// {
	// 	var scene = this.scene;
	// 	if (scene !== null)
	// 	{
	// 		scene.step();
	// 	}
	// 	else
	// 	{
	// 		this.perform();
	// 	}

	// 	this.plan();
	// 	return this.plannedActions.length === 0;
	// },

	// enter : function(loc)
	// {
	// 	this.Locale = loc;
	// 	//this.updateActivities();
	// },

	// isAt : function(loc)
	// {
	// 	return this.Locale === loc;
	// },

	// isInside : function(loc)
	// {
	// 	return this.Locale !== null && this.Locale.isInside(loc);
	// },

	// plan : function()
	// {
	// 	this.plannedActions.length = 0;

	// 	if (this.actions.length > 0)
	// 		return;

	// 	if (!this.hasExplored(this.Locale))
	// 	{
	// 		this.plannedActions.push(Definition.get("Activities.Explore").instantiate([this.Locale], this.context));
	// 	}
	// },

	// accepts : function(quest)
	// {
	// 	this.quests.push(quest);
	// 	quest.accepted(this);
	// 	if (this.Locale !== null)
	// 		this.addQuestActivities(quest);
	// },

	// updateActivities : function()
	// {
	// 	this.questActivities = [];
	// 	this.localActivities = [];

	// 	for (var i = 0; i < this.quests.length; i++) 
	// 	{
	// 		this.addQuestActivities(this.quests[i]);
	// 	};
	// },

	// addQuestActivities : function(quest)
	// {
	// 	context = {};
	// 	context.party = this;

	// 	for (var i = 0; i < quest.goals.length; i++)
	// 	{
	// 		var firstStep = Planner.plan([quest.goals[i]], context);
	// 		if (firstStep === null)
	// 			continue;

	// 		this.addUniqueActivity(this.questActivities, firstStep.action);
	// 	}
	// },

	// addUniqueActivity : function(activities, activity)
	// {
	// 	for (var i = activities.length - 1; i >= 0; i--) 
	// 	{
	// 		if (activities[i].equals(activity))
	// 			return;
	// 	};

	// 	activities.push(activity);
	// },

	// getActivities : function()
	// {
	// 	return this.questActivities.concat(this.localActivities);
	// },

	// hasExplored : function(Locale)
	// {
	// 	return false;
	// },

	// update : function(entry)
	// {
	// 	if (this.scene)
	// 	{
	// 		this.scene.update(entry);
	// 	}		
	// }
}