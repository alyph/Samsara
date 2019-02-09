// Definition.register(
// {
// 	base : ActivityDefinition,
// 	group : "Activities"
// },

// {
// 	TravelTo:
// 	{
// 		title : "Travel",
// 		params : $p("origin", "destination"),
// 		effects : [ "Goals.PartyIsAt(destination)" ],
// 		restrictions : { origin : $e("destination.getPotentialTravelOrigins(party)") },
// 		preconditions : [ "Goals.PartyIsAt(origin)" ]
// 	},

// 	Explore:
// 	{
// 		title : "Explore",
// 		params : $p("locale"),
// 		effects : [ "Goals.PartyHasExplored(locale)" ],
// 		preconditions : [ "Goals.PartyIsAt(locale)" ],
// 		seq : $obj(Seq_Explore)
// 	}

// 	/*
// 	Visit:
// 	{
		
// 	},

// 	Explore:
// 	{
// 		description: "Explore the area"
// 	},

// 	BulletinBoard:
// 	{
// 		description: "What's on the bulletin board?"
// 	},

// 	VisitBar:
// 	{
// 		description: "Enter the tavern"
// 	},

// 	VisitAgora:
// 	{
// 		description: "Shopping at the Aqora"
// 	}
// 	*/
// })