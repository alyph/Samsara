Definition.register(
{
	"Goals.Base":
	{
		$base : GoalType
	}
});

Definition.register(
{
	base : "Goals.Base",
	group : "Goals"
},
{
	PartyIsAt:
	{
		params : $p("locale"),
		expression : $e("party.isAt(locale)")
	},

	PartyHasExplored:
	{
		params : $p("locale"),
		expression : $e("party.hasExplored(locale)")
	}
});