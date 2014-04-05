var BATTLE_MARGIN_X = 10;
var BATTLE_MARGIN_Y = 10;
var COMPANY_WIDTH = 800;
var COMPANY_HEIGHT = 300;



var Battle = Class(
{
	constructor : function(game, party, target)
	{
		this.friendlies = party.members;
		this.enemies = target.members;
	},

	start : function()
	{

	},

	next : function()
	{

	}
});

var Battleground = Class(
{
	constructor: function(game)
	{
		this.game = game;
		this.node = $.playground().addGroup("battleground", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		this.node.addSprite("battleBack", {animation: Sprites.BattleBack, width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});

		this._friendlies = new CardArea(this, BATTLE_MARGIN_X, PLAYGROUND_HEIGHT - BATTLE_MARGIN_Y - COMPANY_HEIGHT, COMPANY_WIDTH, COMPANY_HEIGHT);
		this._enemies = new CardArea(this, BATTLE_MARGIN_X, BATTLE_MARGIN_Y, COMPANY_WIDTH, COMPANY_HEIGHT);

		this.node.hide();
	},

	playBattle : function(battle, callback)
	{
		this._battle = battle;

		this._friendlies.replaceCards(battle.friendlies);
		this._enemies.replaceCards(battle.enemies);

		battle.start();
		this.node.show();
	},

	next : function()
	{
		this._battle.next();

	}
});
