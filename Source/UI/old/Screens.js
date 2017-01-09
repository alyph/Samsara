UI.Template(
["baseScreen", { position: "absolute", width : "100%", height: "100%" }
]);

UI.Template(
["gameScreen", { $base: "baseScreen" },
	["image#boardBack", { position: "absolute", width: "100%", height: "100%" }],
	["image#sceneBack", { position: "absolute", useOriginalSize: true, right: 0 }],
	["grid#center", { position: "absolute", display: "flex", width: "100%", "flex-direction": "column" },
		["label#title", {}],
		["label#features", {}],
		["grid", { flex: "1" }],
		["label#prompts", {}, ]
	]/*,
	["scene#scene", { position: "absolute", width: "100%" }]*/
]);

