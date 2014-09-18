var Character = Class(Entity,
{
	constructor : function()
	{
		Character.$super.call(this);

		this.name ="nameless";
		this.sprite = null;
		this.size = "normal";
	},

	getTitle : function()
	{
		return this.name;
	},

	getImage : function()
	{
		return Sprites[this.sprite];
	},

	getSize: function()
	{
		return this.size;
	} 
});