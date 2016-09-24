
111 -> a

b, x = max(a, b)
b = o1, x = o2 : foo(a = 1 b = 2).a 

(x, y) => (x + y, x *x) => 

if ()
{

	(c, d) => (a, b, >>foo) => bar 

	foo => (x -> x * x) => ((dho, a) => b => c, a)
	get => (>>a, b, (c, d) => foo) =>  bar = a, b

	bar ?good =>(a, b) cur  :bad  kel

	=> (foo > bar) ?
	success:
	{
		=> zel
		(x: good, (out:, a)  )

	}
	else:
	{

	}

}


$defs("temp_locale", "keyword locale", 
{
	"starting_loc":
	{

	}
});


<event> = <event clause>

<event caluse>:
{
	<event properties>
	trigger = <condition clause>
	option = <option clause>
}

<event properties>:
<bool value keys> = <bool values>
...

<bool value> : yes|no
<condition clause> :
{
	<operators> : <condition clause>
	<bool conditions>
	<int conditions>
	<id conditions>
	<clause conditions>
}

- option = 

<key>:
<events>
<properties>
<conditions>
<operators>
<scopes>
<command>















namespace(
{
	$inst('foo',
	{

	}),
});


Behavior({

	itemTemplate: { type: String, value: "" }

	onclick: function(element, event)
	{
		this.itemTemplate.get(element);	
	}
})

get: function(element)
{
	if (isCustomElement(element))
	{
		if (this.readonly)
		{
			
		}
	}
	else
	{
		return this.parse(element.dataset(this.name));
	}
}

function Behavior.init(props)
{
	attachProperties(props);
	attachEventListners();
}

min(1, 3)
((1 + ( scene is a '')) is )

statement := resolver | predicate
predicate := parameter + keyword + parameter ... |: , )
parameter = resolver + reference + code + ... |: keyword ,  )
resolver = keyword + (predicate, precicate ...)
reference = identifier + . resolver + . resolver ...


predicate: (a, b)
"all
 (
	a is 'good',
	b is 'good',
	min(a.size, b.size) <= 0.0
 )
 (b is good or c is ok)"


 function()
 {
 	return (is(a, good)) && (is(b, good)) && lessOrEqual(min(size(a), size(b)), 0.0);
 };

predicate.eval(a, b);

predicate_bar1: function(a, b)
{
	return predicate_foo(a, b, 'friend');
}

// custom
predicate_foo: function(a, b, c)
{
	a.is(b).s(c);

	// a is b's c
	return all(a.hasRelationWith(b, c), a.isa('food'));
}

// native
predicate_hasRelationWith: function(a, b, c)
{
	return a.hasRelationWith(b, c);
}

/*
a is father of b
b is son of a

a's son is b
*/
// tokenize
// parse predicate
// predicate expression: [keyword]  + parameter expression + [keyword] + ...
// parameter expression : [resolver] + [variable] + [script] + ...   //[operators] + [resolver expression] + [operators] + ...
// resolver expression: identifier + resolver + resolver ...
// resolver: .keyword(parameter expression, ...)

// composite predicate : keyword + ( predicate + , + predicate + ...)

// predicate
//	- statement
//	- eval
//		- native eval -> function(a, b)...
//		- custom eval -> predciate expression

// predicate expression
//	- predicate
//	- params: (plain values, input, resolvers, predicate expressions)

{
	travel:
		$predicate("l, pov",
		"any
		(
			pov.locale.region('s') is neighbor of l,
			l is inside of pov.locale's(region)
		)")

	conditions: 
		"(t, i) =>
		all
		(
			t is part_of with i,
			t isa friendly,
			t's(friend)'s(friend) is a king,
			t has relation part_of with i,

			t.hasKeyword(other('part_of'), 'outdoor'), 
			t.hasrelation(i.s('friend'), 'bordering')
		)"
}


all: 
 static_params: variadic
 [
 	haskeyword:
 		1 dynamic param -> resolver
 	static: 1
 	[
 		outdoor
 	],

 	hasrelation:
 		2 dyanamic (implicit)
 		1 static:
 		[
 			bordering
 		]
 ]








world.definitions = data.load("definitions");
world.entities = data.load("entities");


schema = function(c)
{
  c.list("relations", Locale.Relation, "@type", "@other", ["descriptors", $desc]);
}

$def()
$inst()
$use()
$begin()
$end()

{
	"desc": $v(
	[
		"within @x",
		"bordering @y"
	])
}

record:
{
	name: "",
	base: ??,
	namespace: ??,
	index: -1,
	prototype : {},
	object : null
}

{
	$base: "another object" || constructor,
	key: value
}
copy over key values from base