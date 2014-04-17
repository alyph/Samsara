UI.Template(
["baseScreen", { position: "absolute", width : "100%", height: "100%" }
]);

UI.Template(
["gameScreen", { $base: "baseScreen" },
	["image#background", { position: "absolute", width : "100%", height: "100%" }],
	["board#entities", { itemTemplate: "$card", position: "absolute", width : "100%", height: "100%" }]
]);