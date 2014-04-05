var Narrator = Class(
{
	constructor : function(game, script, scene)
	{
		this._game = game;
		this._table = game.table;
		this._script = script;
		this._scene = scene;

		this._line = -1;

		this._text = "";
	},

	begin : function()
	{
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
			this.write(line.substr(1));
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

	/*
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
	},*/

	stage : function()
	{
		this._table.placeStage();
	},

	enter : function(charName, section)
	{
		section = section || "Others";
		section = Sections[section];

		var character = this._game.characterDeck.getCard(charName);
		this._scene.addActor(character);
		this._table.placeEntity(character, section);
	},

	battle : function(target)
	{
		target = this.context.eval(target);
		this._game.startBattle(this._party, target);
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

	background : function(imageName)
	{
		var image = Sprites[imageName];
		if (image === undefined)
			throw ("Cannot find sprite:" + imageName);

		this._table.changeBackground(image);
	},

	write : function(line)
	{
		var input = {};
		input.line = line;
		input.speaker = this._parseSpeaker(input);
		input.continue = this._parseContinue(input);

		this._addText(input.line);

		if (!input.continue)
		{
			//this.scene.pause(this.continue.bind(this));
			var paragraphs = this._populateParagraphs();						
			this._table.narrate(input.speaker, paragraphs, this.continue.bind(this));
			return false;
		}

		return true;
	},

	_parseContinue : function(input)
	{
		var line = input.line;
		if (line.charAt(line.length - 1) === '^')
		{
			input.line = line.slice(0, -1);
			return true;
		}
		return false;
	},

	_parseSpeaker : function(input)
	{
		var line = input.line;
		if (line.charAt(0) === '#')
		{
			var idx = line.indexOf(":", 1);
			if (idx >= 0)
			{
				input.line = line.slice(idx + 2);
				return line.slice(1, idx);
			}
		}

		return null;
	},

	contextUpdated : function()
	{
		this.scene.refresh();
	},

	_addText : function(text)
	{
		if (this._text.length > 0 && this._text.charAt(this._text.length - 1) !== '\n')
			this._text += ' ';

		this._text += text;
	},

	_populateParagraphs : function()
	{
		var tokens = [];
		var index = 0;
		var prev = 0;
		var text = this._text;
		var l = text.length;
		this._text = "";
		
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

		var paragraphs = [];
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
				paragraphs.push(paragraph);
				paragraph = "";
			}
		}

		if (paragraph.length > 0)
			paragraphs.push(paragraph);

		return paragraphs;
	},	
});


