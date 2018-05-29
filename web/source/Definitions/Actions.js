// Global.ActionsNamespace = "action";
// $defs(Global.ActionsNamespace,
// {
// 	"camp":
// 	{
// 		$base: ActionDefinition,
// 		predicate: $predicate(
// 			"scene is a 'scene.planning'")
// 	},

// 	"explore":
// 	{
// 		$base: ActionDefinition,
// 		predicate: $predicate(
// 			"scene is a 'scene.planning'"),
// 		targetKey: "explore",
// 		customLabel: "Explore"
// 	},

// 	"travel":
// 	{
// 		$base: ActionDefinition,
// 		predicate: $predicate(
// 			"scene is a 'scene.planning'"),
// 		targetKey: "locations",
// 	},

// 	"inspect":
// 	{
// 		$base: ActionDefinition,
// 		predicate: $predicate(
// 			"scene is a 'scene.encounter'"),
// 		targetKey: "pois",
// 		prePerform: function(pov, target) { target.addLink("prev", pov.scene); }
// 	},

// 	"leave":
// 	{
// 		$base: ActionDefinition,
// 		predicate: $predicate(
// 			`any(
// 				scene is a 'scene.planning',
// 				scene is a 'scene.encounter')`),
// 			//"any(scene is a 'scene.encounter', scene is a 'scene.planning')"),
// 		targetKey: "inside",
// 		customLabel: "Leave"
// 	},

// 	"back":
// 	{
// 		$base: ActionDefinition,
// 		predicate: $predicate(
// 			"scene is a 'scene.inspecting'"),
// 		targetKey: "prev",
// 		customLabel: "Back"
// 	}

// });
