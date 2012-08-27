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
		cardTavern : [ 2, 0 ]
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