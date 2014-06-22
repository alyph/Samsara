var Sequence = Class(
{
	$statics:
	{
		STATE_INIT: 0,
		STATE_RUNNING: 1,
		STATE_FINISHED: 2,
	},

	constructor : function()
	{
		this.state = Sequence.STATE_INIT;
	},

	execute : function(context)
	{
		throw ("must override!");
	}
});

var InstantSequence = Class(Sequence,
{
	constructor : function()
	{
		InstantSequence.$super.call(this);
	},

	execute : function(context)
	{
		if (this.state !== Sequence.STATE_INIT)
			throw ("sequence cannot execute more than once!")

		this.executeInternal(context);
		this.state = Sequence.STATE_FINISHED;
		return this.state;
	},

	executeInternal : function(context)
	{
		throw ("must override!");
	}
});

var LatentSequence = Class(Sequence,
{
	constructor : function()
	{
		LatentSequence.$super.call(this);
	},

	execute : function(context)
	{
		if (this.state === Sequence.STATE_FINISHED)
			throw ("sequence cannot start once it's finished!")

		var continued = this.executeInternal(context);
		this.state = continued ? Sequence.STATE_RUNNING : Sequence.STATE_FINISHED;
		return this.state;
	},

	executeInternal : function(context)
	{
		throw ("must override!");
	}
});