Global.ActionsNamespace = "action";
$defs(Global.ActionsNamespace,
{
	"camp":
	{
		$base: ActionDefinition,
		predicate: $predicate(
			"scene is a 'scene.planning'")
	},

	"explore":
	{
		$base: ActionDefinition,
		predicate: $predicate(
			"scene is a 'scene.planning'")
	},

	"travel":
	{
		$base: ActionDefinition,
		predicate: $predicate(
			"scene is a 'scene.planning'")
	},

	"inspect":
	{
		$base: ActionDefinition,
		predicate: $predicate(
			"scene is a 'scene.encounter'"),
		targetKey: "pois"
	},

	"leave":
	{
		$base: ActionDefinition,
		predicate: $predicate(
			"scene is a 'scene.encounter'"),
		targetKey: "inside",
		customLabel: "leave"
	},

});
