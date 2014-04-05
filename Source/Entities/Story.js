var StoryDefinition = Class(
{
	script : ["INSERT STORY HERE"],
	focus : [],
	portrait : "No Portrait",
	background : "No Background",

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
		scene.setBackground(this.def.background);
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

