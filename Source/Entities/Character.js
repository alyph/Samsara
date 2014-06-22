var Character = Class(Actor,
{
	constructor : function()
	{
		Character.$super.call(this);

		this.name ="nameless";
		this.portrait = null;
		this.size = "normal";
	},

	getTitle : function()
	{
		return this.name;
	},

	getImage : function()
	{
		return Sprites[this.portrait];
	},

	getSize: function()
	{
		return this.size;
	} 
});