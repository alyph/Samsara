(function()
{
	head.ready(function()
	{
		Gallery.load(
		[
			{
				image : "../../../Content/Images/CharacterCardsIWD.png",
				w : 192,
				h : 256,
				SwordsmenCaptain : [ 3, 0 ],
				NorthernGuardian : [ 4, 0 ],
				ShadowBlade : [ 5, 0 ],
				Druid : [ 6, 0 ],
				MercenarySkirmisher : [ 7, 0 ],
				BattleCleric : [ 2, 1 ],
				Skald : [ 3, 1 ],
				NorthernBarbarian : [ 4, 1 ],
				ApprenticeMage : [ 5, 1 ],
				Ranger : [ 6, 1 ],
				MercenaryHeavyInfantry : [ 7, 1 ],
				Assassin : [ 0, 2 ],
				TempleAcolyte : [ 4, 2 ],
				ShadowAgent : [ 5, 2 ],
				ElfGuardsman : [ 7, 2 ],
				FemaleApprenticeMage : [ 5, 3 ],
				FemaleWanderer : [ 6, 3 ],
				WarPriestess : [ 3, 4 ],
				BattleMaiden : [ 4, 4 ],
				FemaleBard : [ 6, 4 ]
			},

			{
				image : "../../../Content/Images/CharacterCardsPF.png",
				w : 192,
				h : 256,
				Huntress : [ 5, 0 ],
				JungleBeastMaster : [ 2, 1 ],
				Doctor : [ 5, 1 ],
				Monk : [ 5, 2 ],
				Hunter : [ 1, 3 ],
				Pirate : [ 4, 3 ],
				Gunslinger : [ 1, 4 ]
			}
		]);

		Army = Class(
		{
			constructor : function Army()
			{
				this.general = new General("Władysław II Jagiełło", "Grand Duke of Lithuania and King of Poland");
				this.roster = [];

				this.roster.push(new Unit("Man-At-Arms", "MercenaryHeavyInfantry", 100));
				this.roster.push(new Unit("Man-At-Arms", "MercenaryHeavyInfantry", 80));
				this.roster.push(new Unit("Man-At-Arms", "ShadowBlade", 90));
				this.roster.push(new Unit("Man-At-Arms", "ShadowBlade", 60));
				this.roster.push(new Unit("Knights", "SwordsmenCaptain", 100));
				this.roster.push(new Unit("Knights", "BattleCleric", 100));
			}
		});

		General = Class(
		{
			constructor : function General(name, title)
			{
				this.name = name;
				this.title = title;
			}
		});

		Unit = Class(
		{
			constructor : function Unit(name, portrait, strength)
			{
				this.name = name;
				this.portrait = Gallery.find(portrait);
				this.strength = strength;
			}
		});

		Gallery.ready(function()
		{
			army = new Army();

			$("#screenRoot").html("<army-view></army-view>");				

			//$("#screenRoot")[0].dataset.strange = "asdfs''df\"asdfsaf";

			NewUI.registerTemplates("templates.html").ready(function()
			{
				NewUI.registerTemplates("secondtemplates.html");

				NewUI.ready(function()
				{
					$("#welcomeScreen").fadeTo(100,0,function(){$(this).remove();});
					$("army-view")[0].bind(army);
				});
			});	
		});	
	});

	var libs = "../../Libs/";
	var core = "../../Core/"
	var ui = "../";

	head.load(
		libs + "jquery-2.0.3.js",
		libs + "jsface.js",
		core + "gallery.js",
		ui + "UI.js",
		ui + "behaviors/basic_behaviors.js");	

})();

