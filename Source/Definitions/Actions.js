Global.ActionsNamespace = "action";

$begin(Global.ActionsNamespace);

$def("camp", 
{
	$base: ActionDefinition,
	predicate: $predicate(
		"scene is a 'scene.planning'")
});

$def("explore", 
{
	$base: ActionDefinition,
	predicate: $predicate(
		"scene is a 'scene.planning'")
});


$def("travel", 
{
	$base: ActionDefinition,
	predicate: $predicate(
		"scene is a 'scene.planning'")
});

$end();
