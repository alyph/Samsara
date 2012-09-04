var Message = Class(
{
	constructor : function(text)
	{
		this.text = text;
		this.options = [];
	},

	addOption : function(text, data, caller, handler)
	{
		var option = new Option(
			text,
			arguments.length >= 2 ? arguments[arguments.length-1] : null,
			arguments.length >= 3 ? data : null,
			arguments.length >= 4 ? caller : this
		);
		this.options.push(option);
	},

	addOptions : function(options)
	{
		this.options = this.options.concat(options);
	}
});

var Option = Class(
{
	constructor : function(text, handler, data, caller)
	{
		this.text = text;
		this._handler = handler || null;
		this._data = data || null;
		this._caller = caller || this;
	},

	respond : function(game)
	{
		if (this._handler === null)
			throw ("The option " + this.text + " has no response!");

		this._handler.call(this._caller, game, this._data);
	}
})