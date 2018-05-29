var QuestDefinition = Class(
{
	constructor : function() 
	{
		this.params = null;
		this.goals = [];
	}
});

var Quest = Class(
{
	constructor : function(def, game)
	{
		this.def = def;
		this.game = game;
		this.params = new Params(def.params);
		this.party = null;
		this.goals = [];
	},

	accepted : function(party)
	{
		this.party = party;
		this.generateGoals();
	},

	generateGoals : function()
	{
		var goals = this.def.goals;
		for (var i = goals.length - 1; i >= 0; i--) 
		{
			this.generateGoal(goals[i]);
		};
	},

	generateGoal : function(goalStr)
	{
		var tokens = goalStr.split(/\s*[()]\s*/);
		var goalDef = Definition.get(tokens[0]);

		var permutations = [[]];

		if (tokens.length >= 2)
		{

			tokens = tokens[1].split(/\s*,\s*/);

			for (var i = 0; i < tokens.length; i++) 
			{
				var goalParams = Evaluator.eval(tokens[i], this.params);
				goalParams = isArray(goalParams) ? goalParams : [goalParams];

				var l = permutations.length;
				for (var p = 0; p < l; p++) 
				{
					var permu = permutations.shift();

					for (var n = 0; n < goalParams.length; n++)
					{
						permutations.push(permu.concat(goalParams[n]));
					};
				};

			};			
		}

		for (var i = 0; i < permutations.length; i++) 
		{
			var goal = goalDef.instantiate(permutations[i], new StateValue(true));
			this.goals.push(goal);
		};		
	}
});

