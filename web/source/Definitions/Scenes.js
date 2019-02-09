
// $defs("scene", 
// {
// 	"planning":
// 	{
// 		$base: Scene,

// 		populator: function()
// 		{
// 			var locale = this.pov.locale;
// 			var area = locale.desc.query(Archive.get("keyword.part_of")).other();
// 			if (area !== null)
// 			{
// 				this.pois.push(new PointOfInterest(area));

// 				var neighbors = area.desc.query(Archive.get("keyword.bordering")).others();
// 				for (var i = 0; i < neighbors.length; i++) 
// 				{
// 					var poi = new PointOfInterest(neighbors[i]);
// 					this.pois.push(poi);
// 				}
// 			}
// 		}
// 	},

// 	"encounter":
// 	{
// 		$base: Scene
// 	},

// 	"inspecting":
// 	{
// 		$base: Scene,
// 		backdrop: function() { return this.prev.backdrop(); }
// 	},

// 	"exploring":
// 	{
// 		$base: Scene
// 	}
// });

// $defs("randomscene", "scene", {

// 	"void_walkers":
// 	{
// 		$base: "encounter",
// 		label: "Void Walkers",
// 		description: "random encounter void walker"
// 	},

// 	"harpies":
// 	{
// 		$base: "encounter",
// 		label: "Harpies",
// 		description: "random encounter harpies"
// 	},

// 	"hollow_dreamers":
// 	{
// 		$base: "encounter",
// 		description: "random encounter dreamers"
// 	},

// 	"obelisk":
// 	{
// 		$base: "encounter",
// 		description: "random encounter obelisk"
// 	},

// 	"hollow_breach":
// 	{
// 		$base: "encounter",
// 		description: "random encounter hollow"
// 	},

// 	"shadow_path":
// 	{
// 		$base: "encounter",
// 		description: "random encounter shadow"
// 	},

// 	"hidden_stash":
// 	{
// 		$base: "encounter",
// 		description: "random encounter stash"
// 	},

// 	"abadoned_lab":
// 	{
// 		$base: "encounter",
// 		description: "random encounter lab"
// 	},

// 	"watch_tower":
// 	{
// 		$base: "encounter",
// 		description: "random encounter tower"
// 	},

// 	"blood_totems":
// 	{
// 		$base: "encounter",
// 		description: "random encounter totems"
// 	},

// 	"runaway_acolyte":
// 	{
// 		$base: "encounter",
// 		description: "random encounter acolyte"
// 	}
// });

// $insts("gamescene", "scene randomscene",
// {
// 	"iron_march":
// 	{
// 		$base: "planning",
// 		label: "Iron March",
// 		portrait: $sprite("scene_iron_march"),
// 		description: "You are in the moutainous wildness of Iron March. This land is full of Ellirium mines and its population is centered at the ancient fortress of Alamut.",
// 		scenes:
// 		{
// 			locations: ["@alamut", "@caravan_ambush"],
// 			explore: ["@explore_iron_march"]
// 		}
// 	},

// 	"explore_iron_march":
// 	{
// 		$base: "exploring",
// 		description: "Exploring iron march.",
// 		script:
// 		[
// 			{
// 				type: "RandomScene",
// 				scenes: ["@void_walkers", "@harpies", "@hollow_dreamers", "@obelisk", "@hollow_breach", "@shadow_path", "@hidden_stash", "@abadoned_lab", "@watch_tower", "@blood_totems", "@runaway_acolyte"]
// 			}
// 		]
// 	},

// 	"alamut":
// 	{
// 		$base: "planning",
// 		label: "Alamut",
// 		description: "Alamut is one of the most ancient fortresses, dated back to the time before the Age of Creation. Located on the top of Mt. Diialam, its interior is surprisingly spatious and house a population of a small city.",
// 		scenes:
// 		{
// 			inside: ["@iron_march"]
// 		}
// 	},

// 	"caravan_ambush":
// 	{
// 		$base: "encounter",
// 		label: "Ambush Site",
// 		description: "The caravan that was ambushed by the GOBLINs now lies in the middle of the road. Corpses of merchants and guards mingled with bits and pieces of merchandise are everywhere.",
// 		backdrop: $sprite("mountain_pass"),
// 		scenes:
// 		{
// 			pois: ["@destroyed_caravan", "@corpses", "@GOBLINs"],
// 			inside: ["@iron_march"]
// 		}
// 	},

// 	"destroyed_caravan":
// 	{
// 		$base: "inspecting",
// 		label: "Caravan Remains",
// 		description: "The caravan was destroyed during the battle. Goods and supplies spread all over the place.",
// 		portrait: $sprite("scene_ambushed_caravan")
// 	},

// 	"corpses":
// 	{
// 		$base: "inspecting",
// 		label: "Corpses",
// 		description: "Guards and merchants are all dead, their corpses lying motionlessly."
// 	},

// 	"GOBLINs":
// 	{
// 		$base: "inspecting",
// 		label: "Dead GOBLINs",
// 		description: "GOBLINs attacked the caravan. Many were killed, but some escaped with what they came for."
// 	}
// });