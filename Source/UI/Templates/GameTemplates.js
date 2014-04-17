UI.Template(
["card", { padding : 2 },
	["image", { $binding: "getImage()", width: 144, height: 192 }],
	["label#title", { $binding: "getTitle()", position: "absolute" }],
	["label#desc", {}]
]);

UI.Template(
["board", { $element: UI.Board }
]);