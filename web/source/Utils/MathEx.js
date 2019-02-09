'use strict';

/*global MathEx: true*/
/*exported MathEx*/
var MathEx =
{
	randomItem: function (array)
	{
		if (!array.length)
			return null;

		return array[Math.floor(array.length * Math.random())];
	},

	randomInt: function (start, end)
	{
		return start + Math.floor((1 + end - start) * Math.random());
	},

	randomNumber: function (start, end)
	{
		return start + (end - start) * Math.random();
	},

	clamp: function(num, min, max)
	{
		return Math.min(Math.max(num, min), max);
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

	equals(other)
	{
		return this.x === other.x && this.y === other.y;
	}

	copy(other)
	{
		this.x = other.x;
		this.y = other.y;
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
