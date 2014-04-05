var CardPiece = Class(
{
	$statics:
	{
		nextId : 0
	},

	constructor : function(table, card, width, height)
	{
		this._table = table;
		this._id = CardPiece.nextId++;
		this.width = width || CARD_WIDTH;
		this.height = height || CARD_HEIGHT;
		this.visible = false;
		this.node = null;
		this._card = null;
		this._def = null;
		this._speechBubble = null;

		if (card)
			this.changeCard(card);
	},

	getCard : function()
	{
		return this._card;
	},

	changeCard : function(card)
	{
		if (this._card !== card)
		{
			this._card = card;
			this._def = card.definition;

			if (this.visible)
				this._refreshContent();

			this._onCardChanged();
		}
	},

	show : function(x, y)
	{
		if (!this.visible)
		{
			if (this.node == null)
			{
				this._initNode();
			}
			else
			{
				this._refreshContent();
				this.node.show();
			}

			this.visible = true;
		}

		if (arguments.length >= 2)
			this.moveTo(x, y);
	},

	hide : function()
	{
		if (this.visible)
		{
			if (this.node != null)
				this.node.hide();//.remove();

			this.visible = false;
		}
	},

	move : function(x, y)
	{
		this.node.xy(x, y, true);
	},

	moveTo : function(x, y)
	{
		this.node.xy(x, y, false);
	},

	text : function(paragraphs, finished)
	{
		if (this._speechBubble === null)
			this._speechBubble = new SpeechBubble(this, this._table);

		this._speechBubble.show(paragraphs, finished);
	},

	destroy : function()
	{
		if (this.node != null)
		{
			this.node.remove();
			this.node = null;
		}
	},

	rect : function()
	{
		var xy = this.node.xy();
		var wh = this.node.wh();
		return { x : xy.x, y : xy.y, w : wh.w, h : wh.h };
	},

	_onCardChanged : function()
	{
	},

	_initNode : function()
	{
		this.node = this._table.node.addGroup("card_"+this._id, {width : this.width, height : this.height});
		var back = $("<div class='cardBack'></div>").appendTo(this.node);
		var sprite = Sprites[this._card.getImage()/*_def.image*/];
		var spriteId = "cardImage_" + this._id;
		var border = 4;
		var iw = this.width - border * 2;
		var ih = this.height - border * 2;

		back.css(
			{
				left : (this.width - back.outerWidth()) / 2,
				top : (this.height - back.outerHeight()) / 2
			});

		this.node.addSprite(spriteId, {animation: sprite, width: iw, height: ih, posx: border, posy: border });

		this._setupImage();

		$("#"+spriteId).css(
			{
				"border-radius" : "6px"
			});


		var label = $("<div class='cardTitle'></div>").attr("id","cardLabel_" + this._id).html(this._card.getTitle()/*_def.title*/).appendTo(this.node);
		var desc = $("<div class='cardDesc'></div>").attr("id","cardDesc_" + this._id).html(this._card.getDesc()/*_def.desc*/).appendTo(this.node);

		this._setupEvents();
	},

	_refreshContent : function()
	{
		$("#"+ "cardImage_" + this._id).css("background-image", "url("+Sprites[this._card.getImage()/*_def.image*/].imageURL+")");
		$("#"+ "cardLabel_" + this._id).html(this._card.getTitle()/*_def.title*/);
		$("#"+ "cardDesc_" + this._id).html(this._card.getDesc()/*_def.desc*/);
		this._setupImage();
	},

	_setupImage : function()
	{
		var sprite = Sprites[this._card.getImage()/*_def.image*/];
		var spriteId = "cardImage_" + this._id;
		var image = $("#"+spriteId);
		var iw = image.width();
		var ih = image.height();
		var backSize = Math.round(sprite.domO.naturalWidth / sprite.delta) * iw;
		var offsetX = -Math.round(sprite.offsetx / sprite.delta) * iw;
		var offsetY = -Math.round(sprite.offsety / sprite.distance) * ih;

		image.css(
			{
				"background-size" : backSize + "px",
				"background-position" : offsetX + "px " + offsetY + "px"
			});
	},

	_setupEvents : function()
	{
		this.node.click(this._clicked.bind(this));
	},

	_clicked : function(e)
	{
		e.stopPropagation();
		this._card.clicked();
		Events.trigger(this, "Clicked");
	}
});

var EntityCardPiece = Class(CardPiece,
{
	constructor : function(table, card)
	{
		this._selected = false;
		this.scene = null;
		EntityCardPiece.$super.call(this, table, card);
	},

	selected : function()
	{
		this._selected = true;
	},

	unselected : function()
	{
		this._selected = false;
	},

	_clicked : function()
	{
		if (this._selected)
		{
			this.unselected();
			this._table.removeEntity(this);
		}
		else
		{
			this.scene.refresh();
			this.selected();
			this._table.placeEntity(this);
		}
	},

	_onCardChanged : function()
	{
		EntityCardPiece.$superp._onCardChanged.call(this);
		this.scene = this._generateScene();
	},

	_generateScene : function()
	{
		var context = {};
		var scene = new Scene(context);
		scene.portrait = this._card;
		scene.setPortrait(this._card.definition.name)
		scene.addText(this._card.definition.detailed);

		var activities = this._card.activities;
		var activityClicked = this._activityClicked.bind(this);

		for (var i = 0; i < activities.length; i++)
		{
			var activity = activities[i];
			scene.addChoice(activity.displayName, activityClicked, { activity : activity });
		}

		return scene;
	},

	_activityClicked : function(data)
	{
		this._table.removeEntity(this);
		this._table.game.currentParty.startActivity(data.activity, this._card);
	}
});

