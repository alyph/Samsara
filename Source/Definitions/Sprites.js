var SpriteDefinitions =
[
	{
		image : "Content/Images/Background.png",
		background : [ 0, 0 ]
	},

	{
		image : "Content/Images/BattleBack.png",
		BattleBack : [ 0, 0 ]
	},

	{
		folder : "Content/Images/Backgrounds",
		extension : "png",
		Back_Arena : "Arena",
		Back_Ambush : "Ambush",
		Back_HeartMountain : "HeartMountain",
		Back_Pyre : "Pyre",
		Overlay_Stage : "Stage",
		Back_Theme: "ThemeDark"
	},

	{
		folder : "Content/Images/Locations",
		extension : "png",
		Loc_Camp : "Camp",
		Loc_Outpost : "Outpost",
		Loc_FieldShards : "FieldShards",
		Loc_PillarPrimordia : "TwinPillars",
		Loc_AeonMatrix: "AeonMatrix",
		Loc_Marsh: "Marsh",
		Loc_Forest: "Forest",
		loc_primal_forest: "PrimalForest",
		mountain_pass: "mountain_pass"
	},

	{
		image : "Content/Images/heroes.png",
		w : 384,
		h : 456,
		hero_paladin:					[2, 0],
	},

	{
		image : "Content/Images/scenes.png",
		w : 384,
		h : 256,
		scene_ambushed_caravan:			[0, 0],
		scene_iron_march: 				[1, 0]
	},

	{
		image : "Content/Images/SpritesSmall.png",
		w : 64,
		h : 64,
		Cha_Acolyte:					[1, 0],
		Cha_Warlord:					[2, 0],
		Cha_Deva:						[4, 0],
		Cha_GoblinSwordsman:			[5, 0],
		Cha_GoblinSpearman:				[6, 0],
		Cha_GoblinArcher:				[7, 0],
		Cha_GoblinRider:				[0, 1]	
	},

	{
		image : "Content/Images/CharacterCards.png",
		w : 192,
		h : 256,
		Cha_YoungMechKnight:			[0, 0],
		Cha_FemaleBattleMage:			[1, 0],
		Cha_MaskedArmorLeader:			[2, 0],
		Cha_OorgolianSoldier:			[3, 0],
		Cha_ChanceMoth:					[7, 1],
		Cha_MerkadianSoldier:			[4, 3],
		Cha_Murden:						[6, 1]
		/*
		cardBarbarian : [ 0, 0 ],
		cardBard : [ 1, 0 ],
		cardRogue : [ 2, 0 ],
		cardSorceress : [ 3, 0 ],
		cardWizard : [ 4, 0 ],
		Stalker : [ 4, 2 ]*/
	},

	{
		image: "Content/Images/CharacterCardsLarge.png",
		w : 240,
		h : 300,
		Cha_Callerail:					[0, 0],
		Cha_TravonisUl:					[1, 0],
		Cha_Accelerator:				[3, 0]
	},

	{
		image : "Content/Images/WeaponCards.png",
		w : 144,
		h : 192,
		cardBow : [ 0, 0 ],
		cardShortSword : [ 1, 0 ],
		cardDagger : [ 2, 0 ],
		cardHammer : [ 3, 0 ],
		cardCrossbow : [ 0, 1 ]
	},

	{
		image : "Content/Images/ItemCards.png",
		w : 192,
		h : 256,
		PoisonDagger : [ 1, 2 ],
		Buckler : [ 2, 2 ],
		WarHammer : [ 3, 2 ],
		Spear : [ 4, 2 ],
		LightCrossbow : [ 5, 2 ],
		ShortSword : [ 6, 2 ],
		RodOfMissiles : [ 7, 2 ],
		FireStaff : [ 0, 3 ],
		Pistol : [ 1, 3 ],
		PaddedVest : [ 2, 3 ],
		ChainMail : [ 3, 3 ],
		LeatherJacket : [ 4, 3 ],
		HandfulGold : [ 5, 3 ],
		ShortBow : [ 6, 3 ]
	},

	{
		image : "Content/Images/SceneCards.png",
		w : 192,
		h : 256,
		cardStreets : [ 0, 0 ],
		cardCityGate : [ 1, 0 ],
		cardTavern : [ 2, 0 ],
		cardForestPath : [ 3, 0 ],
		Scouting : [ 4, 0 ],
		Cavern : [ 5, 0 ],
		Outpost : [ 6, 0 ],
		MilitaryBase : [ 7, 0 ],
		AbandonedFactory : [ 0, 1 ],
		AbandonedMansion : [ 1, 1 ],
		TempleRuin : [ 4, 1 ],
		AbandonedMine : [ 7, 1 ],
		BanditsCamp : [ 0, 2 ],
		Bar : [ 1, 2 ],
		WeaponStore : [ 2, 2 ],
		MysticalForest : [ 4, 2 ],
		AbandonedTrainStation : [ 5, 2 ],
		DarkWoods : [ 7, 2 ],
		Arena : [ 1, 3 ],
		Dungeon : [ 2, 3 ]
	},

	{
		image : "Content/Images/MonsterCards.png",
		w : 192,
		h : 256,
		Bear : [ 0, 0 ],
		Behold : [ 1, 0 ],
		Boar : [ 1, 1 ],
		Cyclops : [ 4, 1 ],
		DeathKnight : [ 7, 1 ],
		Succubus : [ 0, 2 ],
		Dracolich : [ 4, 2 ],
		Ghost : [ 2, 3 ],
		Ghoul : [ 3, 3 ],
		Giant : [ 4, 3 ],
		Gnolls : [ 5, 3 ],
		Bugbear : [ 6, 3 ],
		Goblins : [ 7, 3 ],
		Hobgoblins : [ 0, 4 ],
		Harpy : [ 1, 4 ],
		Hydra : [ 2, 4 ],
		Kobolds : [ 5, 4 ],
		Lich : [ 1, 5 ],
		Minotaur : [ 7, 5 ],
		OniNightHunter : [ 3, 6 ],
		OniEnforcer : [ 4, 6 ],
		RakshasaAssassin : [ 2, 7 ],
		Salamander : [ 6, 7 ],
		SkeletalTombGuardian : [ 2, 8 ],
		Spider : [ 6, 8 ],
		Wight : [ 0, 9 ],
		cardSoulReaver : [ 0, 0 ],
		cardBerserkMarauder : [ 1, 0 ],
		cardBerserkChampion : [ 3, 0 ],
		cardBerserkVanguard : [ 4, 0 ],
		cardGreenCorruption : [ 7, 0 ],
		cardTwigjack : [ 0, 1 ],
		cardKillerVine : [ 1, 1 ],
		cardPixieOrphan : [ 2, 1 ],
		cardMadRoot : [ 3, 1 ],
		cardPuppetBorn : [ 5, 1 ],
		cardBlackBear : [ 6, 1 ],
		cardBoarWarrior : [ 7, 1 ],
		cardGiantSpider : [ 0, 2 ],
		cardCynoPriest : [ 1, 2 ],
		cardFungiHarvester : [ 2, 2 ],
		cardWargBerserk : [ 3, 2 ],
		cardCynoAvenger : [ 4, 2 ]
	},

	{
		image : "Content/Images/CharacterCardsIWD.png",
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
		image : "Content/Images/CharacterCardsPF.png",
		w : 192,
		h : 256,
		Huntress : [ 5, 0 ],
		JungleBeastMaster : [ 2, 1 ],
		Doctor : [ 5, 1 ],
		Monk : [ 5, 2 ],
		Hunter : [ 1, 3 ],
		Pirate : [ 4, 3 ],
		Gunslinger : [ 1, 4 ]
	},

	{
		image : "Content/Images/CharacterCardsWW.png",
		w : 192,
		h : 256,
		Wehrmacht : [ 0, 0 ],
		SovietTrooper : [ 1, 0 ],
		SovietForager : [ 2, 0 ],
		Viking : [ 3, 0 ],
		Scavenger : [ 4, 0 ],
		Soldier : [ 5, 0 ],
		Engineer : [ 6, 0 ],
		Schutze : [ 7, 0 ],
		MachineGunner : [ 0, 1 ],
		K9 : [ 1, 1 ]
	},

	{
		image : "Content/Images/CharacterCardsHero.png",
		w : 192,
		h : 256,
		Fighter : 		[ 0, 0 ],
		Caster : 		[ 1, 0 ],
		Dwarf : 		[ 2, 0 ],
		Brienne : 		[ 3, 0 ], 
		Anubis : 		[ 4, 0 ],
		Saruta : 		[ 5, 0 ],
		HK47 : 			[ 6, 0 ],
		HK47_2 :		[ 7, 0 ],
	},

	{
		image : "Content/Images/AbilityCards.png",
		w : 192,
		h : 256,
		Parry : [ 0, 0 ],
		BackStab : [ 1, 0 ],
		MagicShield : [ 2, 0 ],
		ShieldUp : [ 3, 0 ],
		ArmorPiercing : [ 7, 0 ],
		PiercingShot : [ 0, 1 ],
		CripplingBlow : [ 4, 1 ],
		RainOfFire : [ 5, 1 ],
		LightningStrike : [ 0, 2 ],
		Healing : [ 2, 2 ],
		Disarm : [ 4, 2 ],
		AimedShot : [ 5, 2 ],
		PointBlankShot : [ 7, 2 ],
		Charge : [ 0, 3 ],
		Blind : [ 1, 3 ],
		PoisonWeapon : [ 3, 3 ],
		RainOfArrows : [ 4, 3 ]
	},

	{
		image : "Content/Images/ActionCards.png",
		w : 192,
		h : 256,
		MechanicalEye :		[ 0, 0 ]
	},

	{
		image : "Content/Images/Figures.png",
		w : 64,
		h : 64,
		FemaleWarrior :		[ 0, 0 ],
		JackalPriest :		[ 1, 0 ],
		Scholar : 			[ 2, 0 ],
		HK47S :				[ 3, 0 ]
	},

	{
		image : "Content/Images/Locations.png",
		w : 48,
		h : 64,
		Market : 			[ 1, 0 ],
		Library : 			[ 2, 0 ]
	},

	{
		image : "Content/Images/Locales.png",
		w : 128,
		h : 96,
		loc_barren :			[ 1, 0 ],
		loc_night_forest :		[ 2, 0 ],
		loc_rocky :				[ 3, 0 ],
		loc_desert_rock :		[ 4, 0 ],
		loc_mountain :			[ 5, 0 ],
		loc_oriental_temple :	[ 6, 0 ],
		loc_forest :			[ 7, 0 ]
	},

	{
		image : "Content/Images/Activities.png",
		w : 192,
		h : 256,
		Explore :			[ 0, 0 ]
	}
];
/*
var Sprites = new (function()
{
	var images = {};
	var imagesTotal = 0;
	var imagesLoaded = 0;
	var finishedHandler = null;

	var Sprite = Class(
	{
		constructor : function()
		{
			this.imageClass = "";
			this.width = 0;
			this.height = 0;
			this.offsetx = 0;
			this.offsety = 0;
		}
	});

	var ImageInfo = Class(
	{
		constructor : function()
		{
			this.url = "";
			this.className = "";
			this.DOM = null;
		}
	});

	this.init = function(finished)
	{
		for (var i = 0; i < SpriteDefinitions.length; i++)
		{
			var def = SpriteDefinitions[i];
			if (def.folder !== undefined)
			{
				this.loadFolder(def);
			}
			else
			{
				this.loadSpriteSheet(def);
			}
		}

		loadImages(finished);
	};

	this.loadFolder = function(def)
	{
		var folder = def.folder;
		var ext = def.extension;

		for (var name in def)
		{
			if (name === 'folder' || name === 'extension')
				continue;

			if (def.hasOwnProperty(name))
			{
				var imgName = def[name];
				var url = folder + "/" + imgName + "." + ext;
				var info = addImage(imgName, url);

				var sprite = new Sprite();
				sprite.image = info;
				this.addSprite(name, sprite);
			}
		}
	};

	this.loadSpriteSheet = function(def)
	{
		var image = def.image;
		var w = def.w;
		var h = def.h;

		var imgName = extractFileName(image);
		var info = addImage(imgName, image);

		if (w !== undefined && h === undefined)
			h = w;

		for (var name in def)
		{
			if (name === 'image' || name === 'w' || name === 'h')
				continue;

			if (def.hasOwnProperty(name))
			{
				var sprite = new Sprite();
				sprite.image = info;

				if (w !== undefined)
				{
					var x = def[name][0] || 0;
					var y = def[name][1] || 0;

					sprite.width = w;
					sprite.height = h;
					sprite.offsetx = x * w;
					sprite.offsety = y * h;
				}

				this.addSprite(name, sprite);
			}
		}
	};

	this.addSprite = function(name, sprite)
	{
		if (name in this)
			throw ("cannot add " + name + " to sprites, name already taken!");

		this[name] = sprite;		
	};

	function extractFileName(url)
	{
		return url.split('/').pop().split('.')[0];
	};

	function addImage(name, url)
	{
		var className = ("image_" + name).toLowerCase();

		if (images[className])
			throw ("conflicting image name:" + className);

		var image = new ImageInfo();
		image.url = url;
		image.className = className;

		images[className] = image;
		return image;
	};

	function loadImages(finished)
	{
		imagesTotal = 0;
		imagesLoaded = 0;

		var styleStr = "<style> ";
		for (var name in images)
		{
			var img = images[name]
			var url = img.url;
			styleStr += "." + name + " { background-image: url(" + url + "); } ";
			imagesTotal++;
		}
		styleStr += "</style>";

		$('head').append(styleStr);

		finishedHandler = finished;

		for (var name in images)
		{
			var img = images[name]
			var url = img.url;
			img.DOM = $("<img Src=\"" + url + "\" class=\"" + name + "\" style=\"display:none;\" />").appendTo($("body")).load(imageLoaded)[0];
		}
	}

	function imageLoaded()
	{
		$(this).remove();
		imagesLoaded++;

		if (imagesLoaded >= imagesTotal)
		{
			finishedHandler();
		}
	}

})();*/

