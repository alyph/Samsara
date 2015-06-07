
predicate.eval(a, b);

predicate_bar: function(a, b)
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
// parameter expression : [operators] + [resolver expression] + [operators] + ...
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