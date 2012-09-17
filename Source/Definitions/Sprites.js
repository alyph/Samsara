var SpriteDefinitions =
[
	{
		image : "Content/Images/Background.png",
		background : [ 0, 0 ]
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
		image : "Content/Images/SceneCards.png",
		w : 192,
		h : 256,
		cardStreets : [ 0, 0 ],
		cardCityGate : [ 1, 0 ],
		cardTavern : [ 2, 0 ],
		cardForestPath : [ 3, 0 ]
	},

	{
		image : "Content/Images/MonsterCards.png",
		w : 192,
		h : 256,
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
		image : "Content/Images/CharacterCards.png",
		w : 192,
		h : 256,
		cardBarbarian : [ 0, 0 ],
		cardBard : [ 1, 0 ],
		cardRogue : [ 2, 0 ],
		cardSorceress : [ 3, 0 ],
		cardWizard : [ 4, 0 ]
	}
]

var Sprites =
{
	init : function()
	{
		for (var i = 0; i < SpriteDefinitions.length; i++)
		{
			var def = SpriteDefinitions[i];
			var image = def.image;
			var w = def.w;
			var h = def.h;

			if (w !== undefined && h === undefined)
				h = w;

			for (var name in def)
			{
				if (name === 'image' || name === 'w' || name === 'h')
					continue;

				if (def.hasOwnProperty(name))
				{
					var options = {};
					options.imageURL = image;

					if (w !== undefined)
					{
						var x = def[name][0] || 0;
						var y = def[name][1] || 0;

						options.type = $.gQ.ANIMATION_HORIZONTAL;
						options.delta = w;
						options.distance = h;
						options.offsetx = x * w;
						options.offsety = y * h;
					}

					var anim = new $.gQ.Animation(options);

					if (name in this)
						throw ("cannot add " + name + " to sprites, name already taken!");

					this[name] = anim;
				}
			}

		}
	}
}