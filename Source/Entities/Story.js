var StoryDefinition = Class(
{
	script : ["INSERT STORY HERE"],
	focus : [],
	portrait : "No Portrait",

	constructor : function() {}

});

var Context = Class(
{
	constructor : function(){},

	eval : function(line)
	{
		try
		{
			var r = eval("this." + line);
			if (r === undefined)
				throw ("result is undefined!");
			return r;
		}
		catch (err)
		{
			throw ("cannot evaluate '" + line + "' (" + err + ")");
		}
	}
});

var Story = Class(
{
	constructor : function(game, party, def, pretext)
	{
		this._game = game;
		this._party = party;
		this.def = def;

		var context = new Context();
		context.game = game;
		context.party = party;
		if (pretext !== undefined)
			$.extend(context, pretext);

		this.context = context;

		var scene = new Scene(context);
		scene.focus = def.focus;
		scene.setPortrait(this.def.portrait);
		this.scene = scene;

		this._line = -1;
	},

	begin : function()
	{
		this._script = this.def.script;
		this._line = 0;
		this.run();
	},

	pause : function()
	{

	},

	continue : function()
	{
		this.run();
	},

	run : function()
	{
		while (this.execute()){}
	},

	execute : function()
	{
		if (this._line < 0 || this._line >= this._script.length)
		{
			// TODO: end story
			return false;
		}

		var line = this._script[this._line];
		var token = line.charAt(0);

		if (token === '$')
		{
			this._line++;
			return this.executeCommand(line.substr(1));
		}
		else if (token === '?')
		{
			this.awaitForActions();
			return false;
		}
		else if ((token === '=' || token === '-') && line.charAt(1) === '>')
		{
			this._line++;
			this.newStory(line.substr(2), token === '-');
			return false;
		}
		else
		{
			this._line++;
			return this.write(line);
		}
	},

	executeCommand : function(line)
	{
		var tokens = line.split(/[\s,]+/);
		if (tokens.length <= 0)
			throw ("cannot execute command: " + line);
		var command = tokens[0];
		var handler = this[command];
		if (typeof handler != 'function')
			throw ("cannot find command: " + command);

		var r = handler.apply(this, tokens.slice(1));
		if (r === undefined)
			r = true;
		return r;
	},

	awaitForActions : function()
	{
		this.scene.refresh();
	},

	enter : function(site)
	{
		site = this.context.eval(site);
		var stage = new Stage(this._game, this._party, Core.getDef(site.definition.stage), site);
		stage.populate();
		this.context.site = site;
		this.context.stage = stage;
		this.context.location = this._game.makeCard(new CardInstance(Core.getCard(site.definition.location)));
		this.contextUpdated();
		return true;
	},

	newStory : function(line, sub)
	{
		var name = line;
		var def = Core.findDef(name);
		if (def === null || def.constructor !== StoryDefinition)
			name = this.context.eval(line);

		if (sub)
			this._party.pushStory(name);
		else
			this._party.startStory(name);
	},

	write : function(line)
	{
		var input = {};
		input.line = line;
		input.paused = this._parsePaused(input);

		this.scene.addText(input.line);

		if (input.paused)
		{
			this.scene.pause(this.continue.bind(this));
			return false;
		}

		return true;
	},

	_parsePaused : function(input)
	{
		var line = input.line;
		if (line.charAt(line.length - 1) === '^')
		{
			input.line = line.slice(0, -1);
			return true;
		}
		return false;
	},

	contextUpdated : function()
	{
		this.scene.refresh();
	}
});


var Scene = Class(
{
	constructor : function(context)
	{
		this._context = context;
		this.focus = [];
		this.portrait = null;
		this._portraitName = "";
		this.entities = [];
		this._text = "";
		this.paragraphs = [];
		this.choices = [];
		this.pausedHandler = null;
	},

	refresh : function()
	{
		this._setupPortrait();
		this._populateParagraphs();
		this._populateEntities();
		Events.trigger(this, "SceneUpdated");
	},

	setPortrait : function(name)
	{
		this._portraitName = name;
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
	}
});