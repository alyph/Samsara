'use strict';

var Descriptor = Class(
{
	constructor : function()
	{
		this.keyword = null;
		this.params = null;
	}
});

class Keyword
{
	constructor()
	{
	}
}

var Description = Class(
{
	constructor : function()
	{
		this.descriptors = [];
	},

	query : function(keyword)
	{
		return new DescriptionQueryResult(this, keyword);
	},

	$statics:
	{
		parse: function(binder, record)
		{
			var raw = binder.source;
			var target = binder.target;
			var desc = new Description();
			var l= raw.length;

			if (target.descriptors.length > 0)
				desc.descriptors = target.descriptors.concat();

			for (var i = 0; i < l; i++) 
			{
				var descList = raw[i].split(/\s+/);
				var keywordName = descList[0];
				var keyword = binder.resolveObject(keywordName, record);
				if (keyword === null)
					return undefined;

				var descriptor = new Descriptor();
				descriptor.keyword = keyword;

				var dl = descList.length;
				if (dl > 1)
				{
					descriptor.params = [];
					for (var pi = 1; pi < dl; pi++) 
					{
						var param = descList[pi];
						var num = parseFloat(param);
						if (!isNaN(num))
							param = num;

						descriptor.params.push(param);
					};
				}

				desc.descriptors.push(descriptor);
			};

			return desc;
		}
	}
});

var DescriptionQueryResult = Class(
{
	constructor : function(desc, keyword)
	{
		this.found = [];

		var all = desc.descriptors;
		var l = all.length;

		for (var i = 0; i < l; i++) 
		{
			if (all[i].keyword === keyword)
			{
				this.found.push(all[i]);
			}
		};
	},

	exist : function()
	{
		return this.found.length > 0;
	},

	other : function()
	{
		if (this.found.length === 0)
			return null;

		// implies relationship descriptor
		// and 1 - 1 relation

		var params = this.found[0].params;
		if (params === null || params.length === 0)
			throw ("not 1 to 1 relation.");

		return params[0];
	},

	others : function()
	{
		// 1 - * relationship
		var others = [];
		var l = this.found.length;
		for (var i = 0; i < l; i++) 
		{
			var params = this.found[i].params;
			if (params === null || params.length === 0)
				throw ("not 1 to 1 relation.");

			others.push(params[0]);
		};

		return others;
	}
});