'use strict';

class BaseObject
{
	constructor()
	{
		this.$name = "";
		this.$baseObj = null;
	}

	//$canBeSubObject: true,

	init()
	{
	}

	// isA(cls)
	// {
	// 	var current = this.$class;
	// 	while (current)
	// 	{
	// 		if (current === cls)
	// 			return true;

	// 		current = current.$super;
	// 	}
	// }

	name()
	{
		return this.$name.substr(1);
	}

	isInstance()
	{
		return Archive.isInstance(this);
	}
}

BaseObject.prototype.$canBeSubObject = true;