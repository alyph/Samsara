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
		{
			if (list !== null)
				console.error(`foreach over ${list?list.constructor.name:null}, which is not an array.`);
			list = [];
		}

		let oldLen = instance.components.length;
		let newLen = list.length;
		if (oldLen !== newLen)
		{
			instance.populate(this.itemComp, newLen);
		}

		for (let i = 0; i < newLen; i++) 
		{
			let comp = instance.components[i];
			comp.transferDataByCopy(data); // TODO: instead of transfering all the data, should just pick the ones used in this item
			comp.setDataProp(this.itemProp, list[i]);
			comp.refresh();
		}
	}
});

UI.Directive("map",
{
	clauses:
	[
		{ keyword: "map", expression: "(itemProp%s in list%b)", content: "itemComp" }
	]
},
class MapDirective
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
		{
			if (list !== null)
				console.error(`map over ${list?list.constructor.name:null}, which is not an array.`);
			list = [];
		}

		let existingItemsMap = new Map();
		for (let comp of instance.components)
		{
			if (comp.data)
			{
				existingItemsMap.set(comp.data[this.itemProp], comp);
			}
		}

		// Add new components and at the same time remove existing component from the to-delete map.
		for (let item of list)
		{
			if (!existingItemsMap.delete(item))
			{
				let newComp = instance.append(this.itemComp);
				newComp.setDataProp(this.itemProp, item);
			}
		}

		// Remaining components no longer maps to any item, so remove them.
		for (let comp of existingItemsMap.values())
		{
			instance.remove(comp);
		}

		// Refresh all the components in the instance now.
		for (let comp of instance.components) 
		{
			//comp.transferDataByCopy(data); // TODO: instead of transfering all the data, should just pick the ones used in this item
			comp.refresh();
		}
	}
});

UI.Directive("if",
{
	clauses:
	[
		{ keyword: "if, elif", expression: "(condition%b)", content: "component", group: "clauses" },
		{ keyword: "else", content: "elseComp", isFinal: true }
	]
},
class IfDirective
{
	constructor()
	{
		this.clauses = [];
		this.elseComp = null;
	}

	apply(instance, data)
	{
		let compDef = this.elseComp;
		for (let i = 0; i < this.clauses.length; i++) 
		{
			let clause = this.clauses[i];
			if (clause.condition(data))
			{
				compDef = clause.component;
				break;
			}
		}

		if (compDef)
		{
			instance.populate(compDef, 1);
			let comp = instance.components[0];
			comp.transferData(data);
			comp.refresh();
		}
		else
		{
			instance.populate(null, 0);
		}
	}
});

