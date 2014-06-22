
var TestEncounters =
[
	{
		desc: "Before the Pillars of Primodia stand the last three Sephirot Guardians: Lord Ulm of House Fyrewone, Lady Taenya and young initiate Ser Gallius.",
		environment: "Environments.PillarOfPrimordia",
		targets:
		{
			"Character.Ulm": { name: "Lord Ulm", portrait: "Cha_MaskedArmorLeader" },
			"Character.LadyTaenya": { name: "Lady Taenya, Windweaver", portrait: "Cha_FemaleBattleMage" },
			"Character.SerGallius": { name: "Ser Gallius Bluesun", portrait: "Cha_YoungMechKnight" }
		}
	},

	{
		desc: "An Oorgolian searching party is passing by on their eternal missions.",
		environment: "Environments.AeonMatrix",
		targets:
		{
			"Character.OorgolianSoldier1": { name: "Oorgolian Soldier", portrait: "Cha_OorgolianSoldier" },
			"Character.Accelerator": { name: "Accelerator", portrait: "Cha_Accelerator", size: "large" },
			"Character.OorgolianSoldier2": { name: "Oorgolian Soldier", portrait: "Cha_OorgolianSoldier" },
			"Character.OorgolianSoldier3": { name: "Oorgolian Soldier", portrait: "Cha_OorgolianSoldier" }		
		}
	},

	{
		desc: "A party of Merkadian hunting is on their hunting trip. Interestingly a Murden is among them",
		environment: "Environments.Forest",
		targets:
		{
			"Character.MerkadianSoldier1": { name: "Merkadian Soldier", portrait: "Cha_MerkadianSoldier" },
			"Character.Murden": { name: "Murden", portrait: "Cha_Murden" },
			"Character.MerkadianSoldier2": { name: "Merkadian Soldier", portrait: "Cha_MerkadianSoldier" }
		}
	},

	{
		desc: "You are surprised by a giant Callerail and several Chance Moth surounding it.",
		environment: "Environments.Marsh",
		targets:
		{
			"Character.ChanceMoth1": { name: "Chance Moth", portrait: "Cha_ChanceMoth" },
			"Character.Callerail": { name: "Callerail", portrait: "Cha_Callerail", size: "large" },
			"Character.ChanceMoth2": { name: "Chance Moth", portrait: "Cha_ChanceMoth" }
		}
	}
];


var Seq_Explore = Class(LatentSequence,
{
	constructor : function()
	{
		Seq_Explore.$super.call(this);
	},

	executeInternal : function(context)
	{
		if (context.party.scene !== null)
			context.party.leaveScene();

		console.log("exploring...");

		var encounter = MathEx.randomElementOfArray(TestEncounters);

		var scene = new Scene();
		scene.desc = encounter.desc;
		scene.environment = Definition.get(encounter.environment);

		var world = context.party.world;

		for (var name in encounter.targets) 
		{
			var entity = world.findEntity(name);
			if (entity)
				world.destroy(entity);

			var data = encounter.targets[name];
			data.$base = Character;
			data.type = Actor.Types.NPC;

			var target = world.spawn(name, data);
			target.enterScene(scene);
		};

		context.party.enterScene(scene);

		return false;
	}
});