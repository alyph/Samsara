var MathEx =
{
	randomElementOfArray: function (array)
	{
		return array[Math.floor(array.length * Math.random())];
	},

	randomInt: function (start, end)
	{
		return start + Math.floor((1 + end - start) * Math.random());
	},

	randomNumber: function (start, end)
	{
		return start + (end - start) * Math.random();
	}
}
