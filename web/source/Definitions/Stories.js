
/*
Core.Definitions(StoryDefinition,
{
	Story_Base:
	{
		portrait : ""
	}
});

Core.Definitions("Story_Base",
{
	Story_VisitSite:
	{
		portrait : "_site",
		script :
		[
			"You approached the %_site.definition.title%\n^",
			"$enter _site",
			"=>stage.def.story"
		]
	},

	Story_AttackEnemy:
	{
		portrait : "_company",
		script :
		[
			"You engaged the %_company.definition.title%\n^",
			"$battle _company"
		]
	},

	Story_EnterLocation:
	{
		focus : ["stage.detected()"],
		portrait : "location",

		script :
		[
			"%stage.def.desc%\n",
			"%stage.scouting()%\n",

			"?free",
			"+Leave",[ "$end" ]
		]
	},

	Story_Main:
	{
		focus : ["party.agenda"],
		portrait : "Scouting",

		script :
		[
			"Surrounding the small settlement of Red Pearl is the rolling mountains of Rubicon Valley. " +
				"The land here is hazardous and full of deadly creatures, even the rocks and woods don't seem to be stay in one place, " +
				"they are evasive, telling something unknown. Mankind abandoned the vast land into the small valley like " +
				"the tide retreated. Now their daily survival relies heavily on the scavenge of the abandoned cities around. " +
				"Anything you can find here is critical to the community, but first of all, you need to live.",

			"?free"
		],

		"!(somethint hapened)" : []
	},

	Story_EncounterBase:
	{
		focus : ["stage.entities"],
		portrait : "encounter",

		script :
		[
			"?%stage.def.desc%\n"
		]
	},

	Story_Assembly:
	{
		script :
		[
			"$show dream.availableTasks",
			
			"# party leader",
			"Personally I would prefer studying in the Great Alexendria Libarary. But still the decision is yours. What tasks do you want me to take?",
			
			"? Nothing for now.", 
			[
				"As you say, I will attempt to gather more information in the mean time." 
				"$end" 
			]
		]
	}
});*/