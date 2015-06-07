/*
Core.Definitions(SceneDefinition,
{
	Scene_Opening:
	{
		script:
		[
			"$background Back_Pyre",
			"Before the realm's last Pyre of Cycle, a party of adventurers gathered...",
			//"$stage",
			"$enter Brienne Party",
			"#Brienne: Alas, we are here once again.",
			"$enter Anubis Party",
			"#Anubis: The sky rises high... How long do we have left?",
			"$enter Saruta Party",
			"#Saruta: Sooner than we've expected.",
			"$enter HK47",
			"#HK47: 10010011110101001011000100%#!!@",
			"#Saruta: Not the translation module again...",
			"Saruta did some magic and the linguistics circuit is good as new.",
			"#Saruta: The soup you had there looks delicious!",
			"#HK47: Initiating Schrödinger Chamber sequence... (pathetic meat bags...)"
		]
	}
});*/

$begin("scene");

$def("planning", 
{
	$base: Scene,

	populator: function()
	{
		var locale = this.pov.locale;
		var area = locale.desc.query(Archive.get("keyword.part_of")).other();
		if (area !== null)
		{
			this.pois.push(new PointOfInterest(area));

			var neighbors = area.desc.query(Archive.get("keyword.bordering")).others();
			for (var i = 0; i < neighbors.length; i++) 
			{
				var poi = new PointOfInterest(neighbors[i]);
				this.pois.push(poi);
			};
		}
	}
});

$end();
