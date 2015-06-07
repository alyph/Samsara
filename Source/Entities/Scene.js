/*
var SceneDefinition = Class(
{
	script : ["INSERT STORY HERE"],
	
	constructor : function() {}
});

var Sections =
{
	Party : 0,
	Others : 1
};
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scene
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var Scene = Class(BaseObject,
{
	constructor : function()
	{
		Scene.$super.call(this);

		this.populator = null;

		this.pov = null;
		this.pois = [];
	},

	populatePOIs : function()
	{
		if (this.pov.locale === null)
			throw ("pov is not in any locale.");

		if (this.populator === null)
			throw ("no populator!");

		this.pois.length = 0;
		this.populator();
	}
});

// var Scene = Class(
// {
// 	$statics:
// 	{
// 		STATE_IDLE : 0,
// 		STATE_BUSY : 1,
// 		STATE_PAUSED : 2
// 	},

// 	constructor : function()
// 	{
// 		this.environment = null;
// 		this.stage = new Stage();
// 		this.pov = null;
// 		this.state = Scene.STATE_IDLE;
// 		this.actors = [];
// 		this.desc = "";
// 		this.player = null;
// 		this.events = [];
// 		this.time = 0;
// 		this.updater = new LatentUpdater(this, this.beginStep);
// 	},

// 	getDesc : function()
// 	{
// 		return this.desc || this.environment.desc;
// 	},

// 	getState : function()
// 	{
// 		return this.state;
// 	},

// 	preStep : function()
// 	{
// 		return true;
// 	},

// 	update : function(entry)
// 	{
// 		entry.begin();
// 		this.player.showScene(this);
// 		this.updater.start();
// 		//this.progress();
// 	},

// /*
// 	progress : function()
// 	{
// 		// not waiting for response and not finished
// 		while (true)
// 		{
// 			this.step();
// 		};
// 	},*/

// 	beginStep : function()
// 	{
// 		var playerMove = Math.random() <= 0.6;
// 		var cls = playerMove ? ".party" : ".target";
// 		var candidates = this.select(cls, function(){return this.canAct();} );
// 		var chosen = MathEx.randomElementOfArray(candidates.actors);

// 		// Look for group
// 		var actors = chosen.rules.groups !== undefined ?
// 			this.select(function() { return this.wantsToGroupActWith(chosen); }) :
// 			chosen.selected();

// 		// Act
// 		actors.act(playerMove);
// 	},

// 	endStep : function()
// 	{
// 		this.updater.updateFinished(false);
// 	},

// 	initiateCombat : function()
// 	{
// 		var events = [];
// 		var groups = {};

// 		for (var i = this.actors.length - 1; i >= 0; i--) 
// 		{
// 			var actor = this.actors[i];
// 			var group = actor.rules.group;
// 			if (group !== undefined)
// 			{
// 				actor.eventSlot = groups[group];
// 				if (actor.eventSlot)
// 				{
// 					actor.eventSlot.actors.add(actor);
// 					continue;
// 				}
// 			}

// 			actor.eventSlot = new Scene.ActorEvent(new Scene.Selection([actor]));
// 			events.push(actor.eventSlot);

// 			if (group !== undefined)
// 				groups[group] = actor.eventSlot;
// 		};

// 		for (var i = events.length - 1; i >= 0; i--) 
// 		{
// 			this.addEvent(events[i], this.time + MathEx.randomInt(1, 20));
// 		};
// 	},

// 	loadEncounter : function(encounter, party)
// 	{
// 		this.desc = encounter.desc;
// 		this.environment = Definition.get(encounter.environment);

// 		var world = party.world;

// 		for (var name in encounter.targets) 
// 		{
// 			var entity = world.findEntity(name);
// 			if (entity)
// 				world.destroy(entity);

// 			var data = encounter.targets[name];
// 			data.$base = Character;
// 			//data.type = Actor.Types.NPC;

// 			var rules = data.rules;
// 			delete data["rules"];

// 			var target = world.spawn(name, data);
// 			var actor = new Scene.Actor(target, "target", rules);
// 			this.addActor(actor);
// 			//scene.enter(target, targetsGroup);
// 			//target.enterScene(scene);
// 		};

// 		party.scene = this;
// 		for (var i = 0; i < party.members.length; i++) 
// 		{
// 			var member = party.members[i];
// 			var rules = {};
// 			rules.row = member.position === "front" ? 0 : 1;
// 			var actor = new Scene.Actor(member, "party", rules);
// 			this.addActor(actor);
// 		};

// 		this.select(".party").move({formation: "row", anchor: "left", facing: "right"});
// 		this.select(".target").move({formation: "row", anchor: "right", padding: 0.5});

// 		this.player = party.player;
// 	},

// 	addActor : function(actor)
// 	{
// 		if (actor.scene !== null)
// 			throw ("cannot add actor already in a scene");

// 		actor.scene = this;
// 		this.actors.push(actor);
// 	},

// 	removeActor : function(actor)
// 	{
// 		if (actor.scene !== this)
// 			throw ("the actor is not in the scene, cannot remove!");

// 		var index = this.actors.indexOf(actor);
// 		if (index < 0)
// 			throw ("cannot find the actor to remove");

// 		actor.scene = null;
// 		this.actors.splice(index, 1);
// 	},

// 	select : function(selector)
// 	{
// 		var filters = [];
// 		var l = arguments.length;
// 		for (var i = 0; i < l; i++) 
// 		{
// 			var filter = arguments[i];
// 			if (filter[0] === '.')
// 			{
// 				var cls = filter.substr(1);
// 				filter = function() { return this.isA(cls); }
// 			}

// 			filters.push(filter);
// 		};

// 		var selected = [];

// 		this.actors.forEach(function(a)
// 		{
// 			for (var i = 0; i < l; i++) 
// 			{
// 				if (!filters[i].call(a))
// 					return;
// 			};

// 			selected.push(a);
// 		});

// 		return new Scene.Selection(selected);
// 	},

// 	addEvent : function(event, time)
// 	{
// 		if (event.scene !== null)
// 			throw ("cannot add event if the event is already in a scene!");

// 		event.scene = this;

// 		var start = 0;
// 		var end = this.events.length;

// 		for (var i = 0; i < end; i++) 
// 		{
// 			var curTime = this.events[i].time;
// 			if (curTime < time)
// 			{
// 				start = i + 1;
// 			}
// 			else if (curTime > time)
// 			{
// 				end = i;
// 				break;
// 			}
// 		};

// 		if (start < end)
// 		{
// 			start = MathEx.randomInt(start, end); 
// 		}

// 		this.events.splice(start, 0, event);
// 	},

// 	removeEvent : function(event)
// 	{
// 		if (event.scene !== this)
// 			throw ("cannot remove if the event is not in the scene!");

// 		event.scene = null;
// 		var idx = this.events.indexOf(event);
// 		if (idx >= 0)
// 			this.events.splice(idx, 1);
// 	},

// 	moveEvent : function(event, time)
// 	{
// 		this.removeEvent(event);
// 		this.addEvent(event, time);
// 	}
// });

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Scene.Actor
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Scene.Actor = Class(
// {
// 	constructor : function(entity, classes, rules)
// 	{
// 		this.scene = null;
// 		this.cluster = null;
// 		this.entity = entity;
// 		this.rules = rules || {};
// 		this.loc = [-1, -1, 0, 0];
// 		this.facing = "left";
// 		this.width = 1;
// 		this.height = 1;
// 		this.classes = classes.split(/\s+/);
// 		this.eventSlot = null;
// 	},

// 	x : function()
// 	{
// 		return this.loc[0] + this.loc[2];
// 	},

// 	y : function()
// 	{
// 		return this.loc[1] + this.loc[3];
// 	},

// 	isOnStage : function()
// 	{
// 		return this.loc[0] >= 0;
// 	},

// 	sprite : function()
// 	{
// 		return this.entity.getImage();
// 	},

// 	isA : function(cls)
// 	{
// 		return (this.classes.indexOf(cls) >= 0);
// 	},

// 	enterCluster : function(cluster)
// 	{
// 		if (cluster === null)
// 			throw ("cannot enter null cluster!");

// 		if (cluster.scene !== this.scene)
// 			throw ("cannot move to cluster not in the same scene!");

// 		if (this.cluster === cluster)
// 			return;

// 		if (this.cluster !== null)
// 			this.cluster.removeActor(this);

// 		this.cluster = cluster;
// 		this.cluster.addActor(this);
// 	},

// 	selected : function()
// 	{
// 		return Scene.Selection([this]);
// 	},

// 	wantsToGroupActWith : function(other)
// 	{
// 		throw ("not implemented!");
// 	}
// });

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Scene.Selection
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Scene.Selection = Class(
// {
// 	constructor : function(actors)
// 	{
// 		if (!actors.length)
// 			throw("at least one actor in the selection");

// 		this.scene = actors[0].scene;
// 		this.actors = actors;

// 		if (this.scene === null)
// 			throw ("cannot select if the actor is not in the scene!");

// 		for (var i = actors.length - 1; i >= 1; i--) 
// 		{
// 			if (actors[i].scene !== this.scene)
// 				throw("all actors must be in the same scene!");
// 		};
// 	},

// 	add : function(actor)
// 	{
// 		if (actor.scene !== this.scene)
// 			throw("the actor is not in the same scene!");

// 		this.actors.push(actor);
// 	},

// 	act : function(playerMove)
// 	{
// 		var actionInfos = this.populateActions();
// 		if (actionInfos.length === 0)
// 			throw ("no action available!");

// 		if (playerMove)
// 		{
// 			this.scene.player.chooseAction(actionInfos, $handler(this, "onActionChosen"));
// 		}
// 		else
// 		{
// 			this.onActionChosen(MathEx.randomElementOfArray(actionInfos));
// 		}
// 	},

// 	populateActions : function()
// 	{
// 		throw ("not implemented!");
// 	},

// 	onActionChosen : function(chosenActionInfo)
// 	{
// 		throw ("not implemented!");
// 	},

// 	move : function(rules)
// 	{
// 		this.moveToSpace(rules);
// 	},

// 	moveToSpace : function(rules)
// 	{		
// 		var formation = this.makeFormation(rules);
// 		this.enterCluster(new Scene.Cluster(this.scene));
// 		this.scene.stage.placeFormation(formation, rules);
// 	},

// 	makeFormation : function(rules)
// 	{
// 		var formationCls;

// 		if (rules.formation === "row")
// 		{
// 			formationCls =  Scene.RowFormation;
// 		}
// 		else
// 		{
// 			throw ("invalid formation!");
// 		}

// 		var formation = new formationCls(this.actors);
// 		formation.arrange(rules);
// 		return formation;
// 	},

// 	enterCluster : function(cluster)
// 	{
// 		for (var i = this.actors.length - 1; i >= 0; i--) 
// 		{
// 			this.actors[i].enterCluster(cluster);
// 		};
// 	}
// });

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Scene.Cluster
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Scene.Cluster = Class(
// {
// 	constructor : function(scene)
// 	{
// 		this.scene = scene;	
// 		this.actors = [];
// 	},

// 	addActor : function(actor)
// 	{
// 		this.actors.push(actor);
// 	},

// 	removeActor : function(actor)
// 	{
// 		var index = this.actors.indexOf(actor);
// 		if (index >= 0)
// 			this.actors.splice(index, 1);
// 	}
// });

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Scene.Formation
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Scene.Formation = Class(
// {
// 	constructor : function(actors)
// 	{
// 		this.actors = actors.concat();
// 		this.grid = new Grid();
// 	},

// 	arrange : function(rules)
// 	{
// 		throw ("must override.");
// 	}
// });

// Scene.RowFormation = Class(Scene.Formation,
// {
// 	constructor : function(actors)
// 	{
// 		Scene.RowFormation.$super.call(this, actors);
// 	},

// 	arrange : function(rules)
// 	{
// 		this.grid.clear();
// 		var deviation = rules.deviation || 0.5;
// 		var padding = rules.padding || 1;
// 		var space = rules.space || 1;

// 		var rows = [];
// 		for (var i = this.actors.length - 1; i >= 0; i--) 
// 		{
// 			var actor = this.actors[i];
// 			var row = actor.rules.row || 0;			
// 			addToRow(rows, row, actor, padding);
// 		};

// 		var facingRight = rules.facing === "right";
// 		if (facingRight)
// 		{
// 			rows.reverse();
// 		}

// 		var x = 0;		
// 		for (var i = 0; i < rows.length; i++) 
// 		{
// 			var row = rows[i];
// 			if (!row) continue;

// 			var y = -Math.floor(row.height / 2);
// 			var rw = row.width;

// 			for (var a = 0; a < row.actors.length; a++) 
// 			{
// 				var actor = row.actors[a];
// 				var posx = x + (rw - actor.width) / 2 + MathEx.randomNumber(-deviation, deviation);
// 				var posy = y + MathEx.randomNumber(-deviation, deviation);

// 				var data = this.grid.place(actor, posx, posy);
// 				data.facing = facingRight ? "right" : "left";
// 				y += actor.height + padding;
// 			};

// 			x += rw + space;
// 		};

// 		function addToRow(rows, index, actor, padding)
// 		{
// 			var row = rows[index];
// 			if (row)
// 			{
// 				row.width = Math.max(row.width, actor.width);
// 				row.height += actor.height + padding;
// 				row.actors.push(actor); // TODO: randomize (add a util function I guess)
// 			}
// 			else
// 			{
// 				rows[index] =
// 				{
// 					width: actor.width,
// 					height: actor.height,
// 					actors: [actor]
// 				};
// 			}
// 		};
// 	},
// });

// Scene.Event = Class(
// {
// 	constructor : function()
// 	{
// 		this.scene = null;
// 		this.time = 0;
// 	}
// });

// Scene.ActorEvent = Class(Scene.Event,
// {
// 	constructor : function(actors)
// 	{
// 		Scene.ActorEvent.$super.call(this);

// 		this.actors = actors;
// 	}
// });

// var TestEncounters =
// [
// 	{
// 		desc: "You are approached by a goblin warband led by a warg rider.",
// 		environment: "Environments.CloudSerpentHills",
// 		targets:
// 		{
// 			"Character.GoblinThrall1": { name: "Goblin Thrall", sprite: "Cha_GoblinSpearman", rules: { row: 0, group: 1 } },
// 			"Character.GoblinStalker": { name: "Goblin Stalker", sprite: "Cha_GoblinSwordsman", rules: { row: 0, group: 1 } },
// 			"Character.GoblinThrall2": { name: "Goblin Thrall", sprite: "Cha_GoblinSpearman", rules: { row: 0, group: 1 } },
// 			"Character.GoblinWargrider": { name: "Goblin Wargrider", sprite: "Cha_GoblinRider", rules: { row: 1 } },
// 			"Character.GoblinHunter1": { name: "Goblin Hunter", sprite: "Cha_GoblinArcher", rules: { row: 2, group: 2 } },
// 			"Character.GoblinHunter2": { name: "Goblin Hunter", sprite: "Cha_GoblinArcher", rules: { row: 2, group: 2 } }
// 		}
// 	}

// 	/*
// 	{
// 		desc: "Before the Pillars of Primodia stand the last three Sephirot Guardians: Lord Ulm of House Fyrewone, Lady Taenya and young initiate Ser Gallius.",
// 		environment: "Environments.PillarOfPrimordia",
// 		targets:
// 		{
// 			"Character.Ulm": { name: "Lord Ulm", portrait: "Cha_MaskedArmorLeader" },
// 			"Character.LadyTaenya": { name: "Lady Taenya, Windweaver", portrait: "Cha_FemaleBattleMage" },
// 			"Character.SerGallius": { name: "Ser Gallius Bluesun", portrait: "Cha_YoungMechKnight" }
// 		}
// 	},

// 	{
// 		desc: "An Oorgolian searching party is passing by on their eternal missions.",
// 		environment: "Environments.AeonMatrix",
// 		targets:
// 		{
// 			"Character.OorgolianSoldier1": { name: "Oorgolian Soldier", portrait: "Cha_OorgolianSoldier" },
// 			"Character.Accelerator": { name: "Accelerator", portrait: "Cha_Accelerator", size: "large" },
// 			"Character.OorgolianSoldier2": { name: "Oorgolian Soldier", portrait: "Cha_OorgolianSoldier" },
// 			"Character.OorgolianSoldier3": { name: "Oorgolian Soldier", portrait: "Cha_OorgolianSoldier" }		
// 		}
// 	},

// 	{
// 		desc: "A party of Merkadian hunting is on their hunting trip. Interestingly a Murden is among them",
// 		environment: "Environments.Forest",
// 		targets:
// 		{
// 			"Character.MerkadianSoldier1": { name: "Merkadian Soldier", portrait: "Cha_MerkadianSoldier" },
// 			"Character.Murden": { name: "Murden", portrait: "Cha_Murden" },
// 			"Character.MerkadianSoldier2": { name: "Merkadian Soldier", portrait: "Cha_MerkadianSoldier" }
// 		}
// 	},

// 	{
// 		desc: "You are surprised by a giant Callerail and several Chance Moth surounding it.",
// 		environment: "Environments.Marsh",
// 		targets:
// 		{
// 			"Character.ChanceMoth1": { name: "Chance Moth", portrait: "Cha_ChanceMoth" },
// 			"Character.Callerail": { name: "Callerail", portrait: "Cha_Callerail", size: "large" },
// 			"Character.ChanceMoth2": { name: "Chance Moth", portrait: "Cha_ChanceMoth" }
// 		}
// 	}*/
// ];

// var generateRandomEncounter = function(party)
// {
// 	var encounter = MathEx.randomElementOfArray(TestEncounters);

// 	var scene = new Scene();
// 	scene.loadEncounter(encounter, party);
// 	return scene;
// };


