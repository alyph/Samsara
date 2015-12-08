UI.Template(
["card", { $class: "$getSize()" },
	["image", { $binding: "getImage()" }],
	["label#title", { $binding: "getTitle()", position: "absolute" }],
	["label#desc", {}]
]);

UI.Template(
["board", { $element: UI.Board },
	["list#objects", { $binding: "getObjects()", itemTemplate: "$card", position: "absolute", left: 64, right: 64, height: "50%" }]
]);

UI.Template(
["actor", { $class: "$facing" },
	["image#sprite", { $binding: "sprite()", useOriginalSize: true }]
]);

UI.Template(
["stage", { $element: UI.StagePanel, actorTemplate: "$actor" }
]);

UI.Template(
["poi", { },
	["image#portrait", { $binding: "portrait()", useOriginalSize: true }]
]);

UI.Template(
["scene", { $element: UI.SceneViewer, itemTemplate: "$poi" }
]);


