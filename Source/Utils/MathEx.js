'use strict';

/*global MathEx: true*/
/*exported MathEx*/
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
};

/*exported Vector2*/
class Vector2
{
	constructor(x, y)
	{
		this.x = x || 0;
		this.y = y || 0;
	}

	add(other)
	{
		return new Vector2(this.x + other.x, this.y + other.y);
	}

	sub(other)
	{
		return new Vector2(this.x - other.x, this.y - other.y);
	}
}
