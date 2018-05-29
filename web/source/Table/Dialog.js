
var DialogBox = Class(
{
	constructor : function(table, extraClass)
	{
		this._group = table.node;
		this._length = 0;
		this._paragraph = null;
		this._waiting = false;

		this._node =  $("<div class='dialogBox'></div>").appendTo(this._group);
		this._text = $("<div class='dialogText'></div>").appendTo(this._node);
		this._blocker = $("<div class='dialogBlocker'/>").appendTo(this._group);

		if (extraClass)
			this._node.addClass(extraClass);

/*
		this._node.css({
			"position" : "absolute",
			"left" : "" + x + "px",
			"top" : "" + y + "px",
			"width" : "" + width + "px"
		});*/

		this._node.hide();
		this._blocker.hide();

		this._tick = $("<div class='dialogTick'>▼</div>?").appendTo(this._node);
		this._tick.hide();
	},

	show : function(paragraphs, finished)
	{
		this._waiting = false;
		this._tick.hide();

		this._layout(paragraphs);

		this._node.fadeIn(500);

		var dialog = this;
		var clicked = function(e)
		{
			e.stopImmediatePropagation();

			if (dialog._waiting)
				dialog.hide(function(){
					dialog._blocker.off('click', clicked);
					dialog._blocker.hide();
					finished();					
				});
		};
		this._blocker.show();
		this._blocker.on('click', clicked);

		this._text.empty();
		this._paragraphs = paragraphs;
		this._interval = setInterval(this._proceed.bind(this), 20);
		this._seek(-1);
	},

	hide : function(finished)
	{
		this._waiting = false;
		this._node.fadeOut(300, finished);
	},

	isWaiting : function()
	{
		return this._waiting;
	},

	_layout : function(paragraphs)
	{
		this._node.width("")
		this._node.height("auto");

		this._text.empty();
		for (var i = 0; i < paragraphs.length; i++)
			this._text.append("<p>" + paragraphs[i] + "</p>");

		var width = this._node.width();
		this._node.width(width);		

		var height = this._node.height();
		this._node.height(height);

	},

	_seek : function(p)
	{		
		while (p < this._paragraphs.length - 1)
		{
			this._text.append("<p></p>");
			p++;

			if (this._paragraphs[p].length > 0)
				return;
		}

		// no paragraph left, terminate
		clearInterval(this._interval);
		this._finished();
	},

	_proceed: function()
	{
		var children = this._text.children();
		var p = children.length - 1;
		var current = children.last();
		var length = current.html().length;
		var paragraph = this._paragraphs[p];
		length++;

		current.html(paragraph.substr(0, length));

		if (length >= paragraph.length)
			this._seek(p);
	},

	_finished : function()
	{
		this._tick.fadeIn(100);
		this._waiting = true;
	}
});

var SpeechBubble = Class(DialogBox,
{
	constructor : function(figure, table, extraClass)
	{
		SpeechBubble.$super.call(this, table, "speechBubble " + (extraClass || ""));

		this._figure = figure;
		this._node.css({ position : "absolute" });
	},

	_layout : function(paragraphs)
	{
		SpeechBubble.$superp._layout.call(this, paragraphs);

		var width = this._node.width();
		var height = this._node.height();
		var rect = this._figure.rect();

		var x = rect.x + Math.floor((rect.w - width) / 2);
		var y = rect.y - height - 8;//+ rect.h + 8;

		this._node.css(
		{
			"left" : "" + x + "px",
			"top" : "" + y + "px"
		});
	}
});

