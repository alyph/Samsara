/*global Gallery: true*/
/*exported Gallery*/
var Gallery = new (function(global) 
{
	var sprites = {};
	var images = {};
	var imagesTotal = 0;
	var imagesLoaded = 0;
	var finishedHandler = null;

	var Sprite = Class(
	{
		constructor : function()
		{
			this.image = null;
			this.x = 0;
			this.y = 0;
		}
	});

	var ImageInfo = Class(
	{
		constructor : function()
		{
			this.url = "";
			this.className = "";
			this.DOM = null;
			this.width = 0;
			this.height = 0;
			this.tilesx = 1;
			this.tilesy = 1;
		}
	});

	this.load = function(defs)
	{
		if (Object.keys(images).length > 0)
			throw ("multiple call to load is not yet supported.");

		for (var i = 0; i < defs.length; i++)
		{
			var def = defs[i];
			if (def.folder !== undefined)
			{
				loadFolder(def);
			}
			else
			{
				loadSpriteSheet(def);
			}
		}

		loadImages();
	};

	this.ready = function(callback)
	{
		if (finishedHandler !== null)
			throw ("cannot call ready multiple times.");

		finishedHandler = callback;
		checkReady();
	};

	this.find = function(name)
	{
		return sprites[name] || null;
	};

	global.$sprite = function(name)
	{
		var sprite = sprites[name] || null;
		if (sprite === null) // TODO: replace it with an error sprite?
			console.error("The sprite " + name + " does not exist!");
		return sprite; 
	};

	function loadFolder(def)
	{
		var folder = def.folder;
		var ext = def.extension;

		for (var name in def)
		{
			if (name === 'folder' || name === 'extension')
				continue;

			if (def.hasOwnProperty(name))
			{
				var imgName = def[name];
				var url = folder + "/" + imgName + "." + ext;
				var info = addImage(imgName, url);

				var sprite = new Sprite();
				sprite.image = info;
				addSprite(name, sprite);
			}
		}
	}

	function loadSpriteSheet(def)
	{
		var image = def.image;
		var w = def.w;
		var h = def.h;

		var imgName = extractFileName(image);
		var info = addImage(imgName, image);

		if (w !== undefined && h === undefined)
			h = w;

		info.width = w;
		info.height = h;

		for (var name in def)
		{
			if (name === 'image' || name === 'w' || name === 'h')
				continue;

			if (def.hasOwnProperty(name))
			{
				var sprite = new Sprite();
				sprite.image = info;

				if (w !== undefined)
				{
					var x = def[name][0] || 0;
					var y = def[name][1] || 0;

					sprite.x = x;
					sprite.y = y;
				}

				addSprite(name, sprite);
			}
		}
	}

	function addSprite(name, sprite)
	{
		if (name in sprites)
			throw ("cannot add " + name + " to sprites, name already taken!");

		sprites[name] = sprite;		
	}

	function extractFileName(url)
	{
		return url.split('/').pop().split('.')[0];
	}

	function addImage(name, url)
	{
		var className = ("image_" + name).toLowerCase();

		if (images[className])
			throw ("conflicting image name:" + className);

		var image = new ImageInfo();
		image.url = url;
		image.className = className;

		images[className] = image;
		return image;
	}

	function loadImages()
	{
		imagesTotal = 0;
		imagesLoaded = 0;

		var styleStr = "";
		var img;
		for (var name in images)
		{
			img = images[name];
			var url = img.url;
			styleStr += "html /deep/ ." + name + " { background-image: url(" + url + "); } ";
			imagesTotal++;
		}
		styleStr += "";

		var stylesheet = document.createElement("style");
		stylesheet.textContent = styleStr;

		document.querySelector("head").appendChild(stylesheet);

		//$('head').append(styleStr);

		//finishedHandler = finished;

		for (name in images)
		{
			img = images[name];
			img.DOM = new Image();
			img.DOM.id = name;
			img.DOM.addEventListener("load", imageLoaded);
			img.DOM.src = img.url;

			// var img = images[name]
			// var url = img.url;
			// img.DOM = $("<img Src=\"" + url + "\" class=\"" + name + "\" style=\"display:none;\" />").appendTo($("body")).load(imageLoaded)[0];
		}
	}

	function imageLoaded(event)
	{
		imagesLoaded++;

		var img = images[event.target.id];
		if (img)
		{
			if (img.DOM !== event.target)
				throw ("image and DOM mismatch!");

			if (img.width > 0)
			{
				img.tilesx = Math.round(img.DOM.naturalWidth / img.width);
			}

			if (img.height > 0)
			{
				img.tilesy = Math.round(img.DOM.naturalHeight / img.height);
			}
		}
		else
		{
			console.log(`Image was loaded but cannot be found in the list ${event.target.id}, ${event.target.src}`);
		}

		checkReady();
	}

	function checkReady()
	{
		if (imagesLoaded >= imagesTotal && finishedHandler !== null)
		{
			var callback = finishedHandler;
			finishedHandler = null;

			callback();
		}
	}

})(this);

