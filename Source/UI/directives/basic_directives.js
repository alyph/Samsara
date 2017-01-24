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
				console.error(`foreach over ${list.constructor.name}, which is not an array.`);
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
			comp.transferDataByCopy(data);
			comp.setDataProp(this.itemProp, list[i]);
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

