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
		]
	}
});