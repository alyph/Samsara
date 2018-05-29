Queue = Class(
{
	constructor : function()
	{
		this.list = [];
		this.begin = 0;
		this.end = 0;
		this.len = 0;
	},

	enqueue: function(elem)
	{
		if (this.begin === this.end && this.len > 0)
		{
			if (this.begin === 0)
			{
				this.list.push(elem);
			}
			else
			{
				this.list.splice(this.end++, 0, elem);
				this.begin++;	
			}
		}
		else
		{
			this.list[this.end++] = elem;
			if (this.end === this.list.length)
			{
				this.end = 0;
			}
		}

		this.len++;
	},

	dequeue: function()
	{
		if (this.len === 0)
			return undefined;

		var elem = this.list[this.begin++];
		if (this.begin === this.list.length)
		{
			this.begin = 0;
		}

		this.len--;
		return elem;
	},

	isEmpty: function()
	{
		return this.len === 0;
	}
});