var Game = Class(
{
	constructor : function()
	{
	},

	start : function()
	{
		this.world = new World(this);

		this.screen = UI.showScreen("gameScreen");

		this.screen.boardBack.setData(Sprites["Back_Map"]);
		

		//this.table = new Table(this);

		this.player = new Player(this);
		var party = this.player.party;
		//var quest = new Quest(Definition.get("Quests.Exploration"));
		//quest.params.set("region", this.world.getEntity("Region.Greenbelt"));
		//party.accepts(quest);
		party.enter(this.world.getEntity("Locale.PlainStarFall"));

		var scene = new Scene();
		scene.environment = Definition.get("Environments.PlainStarFall");
		party.enterScene(scene);
		party.plan();

		this.playerFinished = this.playerFinished.bind(this);
		this.player.play(this.playerFinished);

		//var world = new World(this);
		//var openingScene = new Scene(this, Core.getDef("Scene_Opening"));
		//openingScene.play();
		//this.currentDream = new Dream(this, world);
		//this.currentDream.begin();


		// var camp = new Location();
		// camp.setDef(Definition.get('Locations.ExpiditionCamp'));

		// var explore = new Activity();
		// explore.setDef(Definition.get('Activities.Explore'));

		// var bulletin = new Activity();
		// bulletin.setDef(Definition.get('Activities.BulletinBoard'));

		// var visitBar = new Activity();
		// visitBar.setDef(Definition.get('Activities.VisitBar'));		

		// var visitAgora = new Activity();
		// visitAgora.setDef(Definition.get('Activities.VisitAgora'));

		// camp.addActivities(explore, bulletin, visitBar, visitAgora);

		// this.enterLocation(camp);
	},

	playerFinished : function()
	{
		this.step();
	},

	step : function()
	{
		var shouldContinue = true;
		var player = this.player;
		var world = this.world;

		while (shouldContinue)
		{
			if (!player.preStep())
				break;

			shouldContinue = player.step();
			world.step();
		}

		player.play(this.playerFinished);
	}

	// initDecks : function()
	// {
	// 	var cards = Core.getCards();
	// 	this.entityDeck = new EntityDeck(this, cards);
	// 	this.encounterDeck = new EncounterDeck(this, cards);
	// 	this.siteDeck = new SiteDeck(this, cards);
	// 	this.companyDeck = new CompanyDeck(this, cards);
	// 	this.unitDeck = new UnitDeck(this, cards);
	// 	this.characterDeck = new CharacterDeck(this, cards);
	// },

	// nextTurn : function()
	// {
	// 	this.endTurn();
	// 	this.turns++;
	// 	this.beginTurn();
	// },

	// beginFirstTurn : function()
	// {
	// 	this.currentParty.drawAgenda(3);
	// },

	// beginTurn : function()
	// {
	// 	if (this.turns === 0)
	// 		this.currentParty.drawEncounter(3);

	// 	this.table.placeEncounters(this.currentParty);

	// 	// player draw cards
	// 	for (var i = 0; i < this.player.handSize; i++)
	// 	{
	// 		this.table.placeInHand(this.player.deck.draw());
	// 	}
	// },

	// endTurn : function()
	// {
	// 	this.table.clearPlayerHand();
	// },

	// enterLocation : function(loc)
	// {
	// 	this.table.setupLocation(loc);
	// },

	// enterScene : function(name)
	// {
	// 	this.scene = new Scene(this, Core.getDef(name));
	// 	this.table.placeScene(scene);
	// 	openingScene.play();
	// },

	// getContext : function()
	// {
	// 	return this.currentParty.story.context;
	// },

	// selectEncounter : function(encounter)
	// {
	// 	this.table.hideEncounters();
	// 	this.currentParty.startEncounter(encounter);
	// },

	// _assembleParty : function()
	// {
	// 	var party = new Party(this);
	// 	for (var i = 0; i < 4; i++)
	// 		party.addMember(this.characterDeck.draw());
	// 	return party;
	// },

	// changeParty : function(party)
	// {
	// 	this.currentParty = party;
	// 	this._updateStory();
	// 	Events.delegate(this.currentParty, "StoryChanged", this._updateStory, this);
	// },

	// _updateStory : function()
	// {
	// 	if (this.currentParty.story === null)
	// 		return;

	// 	this._updateScene();
	// 	Events.delegate(this.currentParty.story.scene, "SceneUpdated", this._updateStory, this);
	// },

	// _updateScene : function()
	// {
	// 	this.table.placeScene(this.currentParty.story.scene);
	// },

	// startBattle : function(party, target, callback)
	// {
	// 	var battle = new Battle(this, party, target);
	// 	this.battleground.playBattle(battle, callback);
	// },

	// setActiveCard : function(card)
	// {
	// 	this.table.placeActiveCard(card);
	// },

	// makeCard : function(cardInst)
	// {
	// 	var card = this._cardPool.makeCard(cardInst);
	// 	card.beginPlay();
	// 	return card;
	// }
});

