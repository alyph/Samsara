var Character = Class(Entity,
{
	constructor : function()
	{
		Character.$super.call(this);

		this.displayName ="nameless";
		this.sprite = null;
		this.size = "normal";
		this.party = null;
		this.pov = null;
	},

	getTitle : function()
	{
		return this.displayName;
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