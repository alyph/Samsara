var Message = Class(
{
	constructor : function(text)
	{
		this.text = text;
		this.options = [];
	},

	addOption : function(text)
	{
		var option = new Option(text);
		this.options.push(option);
	},

	addOptions : function(options)
	{
		this.options = this.options.concat(options);
	}
});

var Option = Class(
{
	constructor : function(text)
	{
		this.text = text;
	}
})