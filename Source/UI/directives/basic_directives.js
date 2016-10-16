UI.Directive("foreach",
{
	clauses:
	[
		{ keyword: "foreach", expression: "(itemProp%s in list%b)", content: "itemComp" }
	]
},
class ForeachDirective
{
	constructor()
	{
		this.list = null;
		this.itemProp = "";
		this.itemComp = null;
	}

	apply(instance, data)
	{
		let list = this.list(data);
		if (!Array.isArray(list))
			list = [];

		let oldLen = instance.components.length;
		let newLen = list.length;
		if (oldLen !== newLen)
		{
			instance.populate(this.itemComp, newLen);
		}

		for (let i = 0; i < newLen; i++) 
		{
			let comp = instance.components[i];
			comp.setDataProp(this.itemProp, list[i]);
			comp.refresh();
		}
	}
});