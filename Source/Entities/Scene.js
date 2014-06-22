/*
var SceneDefinition = Class(
{
	script : ["INSERT STORY HERE"],
	
	constructor : function() {}
});

var Sections =
{
	Party : 0,
	Others : 1
};
*/



var Scene = Class(
{
	$statics:
	{
		STATE_IDLE : 0,
		STATE_BUSY : 1,
		STATE_PAUSED : 2
	},

	constructor : function()
	{
		this.environment = null;
		this.stage = new Stage();
		this.pov = null;
		this.state = Scene.STATE_IDLE;
		this.actors = [];
		this.desc = "";
	},

	getDesc : function()
	{
		return this.desc || this.environment.desc;
	},

	getState : function()
	{
		return this.state;
	},

	preStep : function()
	{
		return true;
	},

	step : function()
	{
		for (var i = 0; i < this.actors.length; i++) 
		{
			this.actors[i].perform();
		};
	},

	addActor : function(actor)
	{
		if (actor.scene !== null)
			throw ("already in a scene, must leave then enter!");
		
		var idx = this.actors.indexOf(actor);
		if (idx >= 0)
			throw ("actor already added!");

		actor.scene = this;
		this.actors.push(actor);
		this.stage.addActor(actor);

		if (actor.elements)
		{
			for (var i = 0; i < actor.elements.length; i++) 
			{
				this.addActor(actor.elements[i]);
			};
		}
	},

	removeActor : function(actor)
	{
		if (actor.scene !== this)
			throw ("cannot remove actor that is not in current scene!");

		actor.scene = null;
		var idx = this.actors.indexOf(actor);
		if (idx >= 0)
			this.actors.splice(idx, 1);

		this.stage.removeActor(actor);

		if (actor.elements)
		{
			for (var i = 0; i < actor.elements.length; i++) 
			{
				this.removeActor(actor.elements[i]);
			};
		}
	}	

/*
	constructor : function(game, def)
	{
		this._game = game;
		this._def = def;




		//this._context = context;
		this.focus = [];
		this.portrait = null;
		this._portraitName = "";
		this.background = null;
		this._backgroundName = "";
		this.entities = [];
		this._text = "";
		this.paragraphs = [];
		this.choices = [];
		this.pausedHandler = null;
	},

	play : function()
	{
		this._game.table.placeScene(this);

		var narrator = new Narrator(this._game, this._def.script, this);
		narrator.begin();
	},

	addActor : function(character)
	{

	},

	refresh : function()
	{
		this._setupPortrait();
		this._setupBackground();
		this._populateParagraphs();
		this._populateEntities();
		Events.trigger(this, "SceneUpdated");
	},

	setPortrait : function(name)
	{
		this._portraitName = name;
	},

	setBackground : function(name)
	{
		this._backgroundName = name;
	},

	addText : function(text)
	{
		if (this._text.length > 0 && this._text.charAt(this._text.length - 1) !== '\n')
			this._text += ' ';

		this._text += text;
	},

	addChoice : function(text, handler, data)
	{
		this.choices.push({
			text : text,
			handler : handler,
			data : data
		});
	},

	isPaused : function()
	{
		return this.pausedHandler !== null;
	},

	pause : function(handler)
	{
		this.pausedHandler = handler;
		this.refresh();
	},

	resume : function()
	{
		var handler = this.pausedHandler;
		this.pausedHandler = null;
		handler();
	},

	_setupPortrait : function()
	{
		if (this._portraitName === "")
		{
			this.portrait = null;
			return;
		}

		if (this.portrait !== null && this.portrait.definition.name === this._portraitName)
			return;

		var game = this._context.game;
		var def = Core.findCard(this._portraitName);
		if (def !== null)
		{
			this.portrait = game.makeCard(new CardInstance(def));
		}
		else
		{
			var portrait = this._context.eval(this._portraitName);
			if (typeof portrait === 'string')
				this.portrait = game.makeCard(new CardInstance(Core.getCard(portrait)));
			else if (portrait.constructor === Card)
				this.portrait = portrait;
			else
				throw ("Cannot setup portrait with name: " + this._portraitName + ", evaluated to " + portrait);
		}
	},

	_setupBackground : function()
	{
		if (this._backgroundName === "")
		{
			this.background = null;
			return;
		}

		this.background = Sprites[this._backgroundName];
		if (this.background !== undefined)
			return;

		var evalName = this._context.eval(this._backgroundName)
		this.background = Sprites[evalName];
		if (this.background === undefined)
			throw ("Cannot find sprite using " + this._backgroundName + " evaluated to " + evalName);
	},

	_populateParagraphs : function()
	{
		var tokens = [];
		var index = 0;
		var prev = 0;
		var text = this._text;
		var l = text.length;
		while (index < l)
		{
			var c = text.charAt(index);
			if (c == '%')
			{
				if (text.charAt(index+1) == '%')
				{
					tokens.push('s' + text.substr(prev, index - prev + 1));
					prev = index += 2;
				}
				else
				{
					if (index > prev)
						tokens.push('s' + text.substr(prev, index - prev));
					prev = index + 1;
					index += 2;
					while (index < l)
					{
						if (text.charAt(index) == '%')
						{
							tokens.push('p' + text.substr(prev, index - prev));
							prev = ++index;
							break;
						}
						index++;
					}
				}
			}
			else if (c == '\n')
			{
				if (index > prev)
					tokens.push('s' + text.substr(prev, index - prev));
				tokens.push('b');
				prev = ++index;
			}
			else
			{
				index++;
			}
		}

		if (prev < text.length && index >= prev)
			tokens.push('s' + text.substr(prev, index - prev + 1));

		this.paragraphs.length = 0;
		var paragraph = "";
		var ctx = this._context;
		l = tokens.length;
		for (var i = 0; i < l; i++)
		{
			var token = tokens[i];
			var tag = token.charAt(0);
			var line = token.substr(1);
			if (tag == 's')
			{
				paragraph += line;
			}
			else if (tag == 'p')
			{
				paragraph += ctx.eval(line);
			}
			else if (tag == 'b')
			{
				this.paragraphs.push(paragraph);
				paragraph = "";
			}
		}

		if (paragraph.length > 0)
			this.paragraphs.push(paragraph);
	},

	_populateEntities : function()
	{
		this.entities.length = 0;
		for (var i = 0; i < this.focus.length; i++)
		{
			this.entities = this.entities.concat(this._context.eval(this.focus[i]));
		}
	}*/
});

