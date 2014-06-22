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