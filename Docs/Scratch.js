// Entity
//	- entity has stats (attack, defense, ap cost, price etc.)
//	- can apply attachment/component to modify stats or even behaviors
//	- can take actions on other entities (contextual)

// Interaction
//	- entity to entity action (e.g. attack, talk, unlock, disarm etc.)
//	- result is determined by comparing certain stats (modified end result)
//	- normal forumla (2x great, >= success, < fail)

// Action
//	- take immediate effect

// Effect
//	- any atomic state change (apply status effect, draw a card, attach attachment, take damage etc.)
//	- usually applied instantly
//	- an action consists of a list of effects
//	- many other things can apply effects (including attachment and status effect etc.)

// Attachment/Component
//	- anything that can be attached to an entity.
//	- exmaple: traits, skills, status effects, equipment, spell, ability etc.
//	- static modifiers (modify some stats consistently)
//	- conditional modifiers (modify based on conditions, context of an interaction)

// Keyword?
//	- similar to traits but bares no modifiers?


// Deck
//	- have data define the initial set of cards (cards are the definitions of entities,
//	  actions, attachments etc.)

// Hand
//	- draw full hand of cards on start.


// Card Type
//	- defines how cards are played
//		* instant: immediate effect and discard
//		* attached: attach to hero card
// 		* summoned: join at party area	
//		* environment: traps and secrets that are attached to the environment
//	- defines possible lasting effects
//		* modifiers
//		* triggered effects
//		* activatble effects

// Skill check
//	- determine the skills to use attack vs defense
//	- multiple skills can be involved (general or special)
//	- gather 

// Traits
//	- traits can be assigned to the cards
//	- some traits have parameters (e.g. spell_damage(2))

// Traits on cards
//	- can allow actions (including active and reactive)
//	- can modify owning card actions
//	- can modify other actions (including adding traits to actions, modifying its params etc.)
//	- can modify skill check

// Traits on actions
//	- can modify the skills used for this action
//	- can modify action specific parameters
//	- simply adding tags so other actions can check

// To define an action
//	- trigger: active or reactive (if reactive, define trigger condition, you can also think the active action is also one type of trigger)
//	- target selection (e.g. hostile? single target or mutli)
//	- skill check (default, many traits can modify this, also it's optional)
//	- outcome (based on skill check, result draw, then play out effects)
//		- normally if the action involves skill check then the result is determined by the result cards
//		- otherwise the action states what the effects are.
//	- an action can be based on another action (inheritance)

// Card effects
//	- immediate effect (activate then discard the card)
//	- activatable effect (when attached, activate to trigger the effect, may or may not has charges)
//	- triggered effect (triggered by certain events, or usually activated when involved in skill check)

// Card template
//	- define traits
//	- can have annonymous traits (sub object) which then can do anything a trait can do. (like modifiers or actions)

// Instanced card
//	- the instanced card may hold tokens
//	- the card may be in the hand, attached or discarded, or played in the play area
//	- tokens may be added when the card is attached.


// concept of mixed inheritance and composition:
// a skill can be a child of another skill
// a skill can also contain other skills
// same for the traits
// THOUGHT: do we need inheritance still, can we just put the in the contained skill list?

// Field areas:
//	- party
//	- encounter
//	- environment

// Plyaer areas:
//	- deck
//	- hand
//	- discard piles


/* eslint ignore:start */


// ways to handle additional attach/equip requirements:
//	- expose targetCondition as an array, and allow modifiers to add entries to the array, all conditions must be met
//	- expose additional requirements as a sginle array parameter, and the modifiers can modify that parameter
//	- [preferred] write any needed requirements in the target condition itself, and expose each requirements config in the parameters
//	  for example, in target condition write expression to check required skill level, and required skill level 
//	  is exposed to the parameters as a map parameter(?), and then the modifiers and insert or modify the values in the map
//	  if the map is empty the condition is ignored.

action = 
{
	// when to play
	timing: in hand or when attached?
	// expression, when set must select a target before executing 
	// (target will be a usable param in effect script)
	// some effect may select target itself (for exmple discarding card),
	// which doesn't require target set here.
	condition: @ (target, instigator) =>
	{
		is_living
		is_hostile
		instigator.is_living //=> { hero && friendly }
	}
	// list of effects to be executed (to be replaced by script)
	effects: []

	// all modifiable parameters (usable in effects script: params.foo)
	params: []
}

card = 
{
	// the root trait
	definition:
	{

	}
}

trait =
{
	// all usable actions
	// this is what's available to modifiers (means enable these actions)
	actions: []

	// all modifiable params (accessible from action effects: card.params.foo)
	params: []

	// parameter values (modifier)
	values: []

	// all inhereited traits
	traits: []
}


globals: world, player
locals: target, card, action // get parms from card or action card.params.foo, action.params.bar
scope: (target, card)

target = .Func.And
{
	conditions:[ .script.is_hero{}, .script.is_friendly{}, 
	.Func.All{ conditions = action.params.requirements } ] // how to handle Reqs.
}
// another way to do additional requirements is to allow
// a trait to modify the actions' target conditions. (e.g. add more conditions)

effect = .Proc.AttachCard
{
	card: "card",
	target: "target"
}

target => { hero && friendly }
 // expand to: (target, self).{ hero && friendly }
 // = (target, self).hero() && (target, self).friendly()

hero => (self) => { self.has_trait("hero") }
friendly => (this, other) => { check_relationship(this, other) == friendly }


// parser state switch
@import otherpkg
@base CommonBase

@namespace MyNamespace
@use OtherNamespace

MyData = //implicit base (CommonBase)
{
	a: 2, b: 3, c: 4
	displayName = Paladin
	smallPortrait : #d>sprite.hero.paladin // a string but pointing to a sprite which will be parsed at load time
	value = 1.10 * 3 - 1
	ref = &refme // reference
	action : #equip.MyAction {// object, based off MyAction
	
	}
	effect: @ =>
	{ 
		attach_card($card, $target) 
	}

	// a block, can be implicit object, but if not found key = token, then it's custom block (can be code)
}


package = { entry }
entry = rules | object_entry | comment
directive = @ identifier (custom parsing)
comment = single_line_comment | multi_line_comment
object_entry = key object
object = class_head { (key_value_pair ,?)* }
class_head = native_class_head | object_reference
native_class_head = $ identifier
object_reference_token = & object_reference
object_reference = env_letter / (identifier .)* identifier
key_value_pair = key value
value = string | expression | object | null | list | object_reference | custom_value
expression = term (operator term)*
term = num | (operator)? term | ( expression )
list = [ (value)* ,? ]
custom_value = @ (custom parsing)

single_line_comment = //.*[line break]
multi_line_comment = /*.**/
key = identifier, : // terminal
identifier
indicators = @&$
operators = +-*/ -- ++ **
punctuation = ()[]{},?.
string 
number






// load the raw record, merge with the bases
// apply the record to the constructed object (property by proerty based on the property type from the constructor)
//	this applies to the sub objects and the sub of the sub objects (recursively...)

var effect = 
`
	attach_card($card, $target)	
	discard_hand_cards(max=2) -> $num_discarded
	deal_damage($target, $num_discarded * 2)

	if not $card.has_trait(hero) and $card.isAlive then

	if card.(
		has_trait(hero) and 
		isAlive and
		cost >= 0
		)
	{
		card.
		{
			play 2 >= 0

			discard 4, 5

		}
	}

	end
`;

var equip_action =
{
	timing: "InHand",
	target: "hero, friendly",
	effect: "attach $card to $target, discard hand cards (max: 2): $x, deal $x*2 damage"
};

var effect_attach =
{
	syntax: "discard at most $x cards",
	impl: "discard hand cards (max: 2)"
};

var fireball =
{
	action:
	{
		traits: "attack, magic, pyromancy, damageThreshold(5)"
	}
};

var sword =
{
	PrimaryAttack: 
	{
		Type: "Attack",
		InstigatorSkills: "MeleeAttack",
		TargetSkills: "MeleeDefense, Armor",
		Quality: +1,
		DamageThreshold: 3
	},

	Traits: "Sword, LightWeapon" // maybe sword trait implies lightWeapon?
};

// another solution <- I like this.
var sword2 =
{
	Traits: "Sword, Quality(+1)" // maybe sword trait implies lightWeapon?
};

var Trait_Sword =
{
	SubTraits: "LightMeleeWeapon, DamageThreshold(3)",
	// another way is just have the traits like "sword master" to check if the action card
	// has a sword trait.
	Modifiers: "modify card action 'MeleeAttack': add instigator skill tag 'sword'" 

};

var Trait_LightMeleeWeapon = 
{
	SubTraits: "MeleeWeapon",
	// just an exmple, may not actually check agility.
	Modifiers: "modify card action 'MeleeAttack': add instigator skill 'Agility'" 
};

var Trait_MeleeWeapon = 
{
	// the card provides this action
	// where do we config this attack.
	// the action will need define bunch of parameters it use
	// and set some default values.
	EnableAction: "MeleeAttack" 
};

var Trait_DamageThreshold =
{
	Params: "Amount",
	Modifiers: "modify card action 'MeleeAttack': +{Amount} DamageThreshold"
};

var hammer =
{
	PrimaryAttack: 
	{
		Type: "Attack",
		InstigatorSkills: "MeleeAttack",
		TargetSkills: "MeleeDefense, Armor",
		Quality: +1,
		DamageThreshold: 4
	},

	Traits: "Hammer, HeavyWeapon"
};

var card =
{
	TriggerEffect: ["when attacked, draw a card"],
	PlayEffect: ["grant concealed to adjacent cards"],
	Modifiers: ["spell damage +1", "attack + 2", "dodge + 1"], // constant or reactive or conditional modifiers
	Traits: ["charge", "concealed"],
	Keywords: ["fly", "ranged"]
};


var ifClause =
{
	keyword: "if elif",
	expression: "(expr%b)",
	content: "component",
	group: "clauses"
};

var elseClause =
{
	keyword: "else",
	content: "elseClause"
};

var foreachClause =
{
	keyword: "foreach",
	expression: "(tem%s in list%b)",
	content: "itemComp"
};

var switchClause =
{
	keyword: "switch",
	expression: "(expr%b)",
};

var caseClause =
{
	keyword: "case",
	expression: "case value%c:",
	content: "component",
	group: "cases"
};

var defaultClause =
{
	keyword: "default",
	expression: ":",
	content: "default"
};



(if)
clauses = [] (binding, component)
elseClause = null;

clause.cond;
clause.component;


(foreach)
itemProp;
list (binding)
itemComp (component)

(switch)

expr (binding)
cases [] (const, component)




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

/* jshint ignore:end */


