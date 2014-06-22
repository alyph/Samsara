UI.Template(
["baseScreen", { position: "absolute", width : "100%", height: "100%" }
]);

UI.Template(
["gameScreen", { $base: "baseScreen" },
	["image#boardBack", { position: "absolute", width: "100%", height: "100%" }],
	["image#background", { position: "absolute", width: 512, height: 224, left: 120, top: 224 }],
	["board#stage", { itemTemplate: "$card", position: "absolute", width: "100%", height: "60%" }],
	["label#description", { position: "absolute", left: 0, top: 500 }],
	["list#actions", { itemTemplate: "$card", position: "absolute", width : "100%", top: 532, bottom: 0, margin: 16 }]
]);
