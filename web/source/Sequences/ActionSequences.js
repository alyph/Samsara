var Seq_Explore = Class(LatentSequence,
{
	constructor : function()
	{
		Seq_Explore.$super.call(this);
	},

	executeInternal : function(context)
	{
		if (context.party.scene !== null)
			context.party.leaveScene();

		console.log("exploring...");

		var scene = generateRandomEncounter(context.party);

		return false;
	}
});

