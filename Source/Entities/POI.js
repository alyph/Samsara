PointOfInterest = Class(
{
	constructor : function(entity)
	{
		this.entity = entity;
	},

	portrait : function()
	{
		var portrait = this.entity.Portrait();
		if (!portrait)
			throw ("invalid portrait.");

		return portrait;
	}
});