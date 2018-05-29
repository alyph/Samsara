'use strict';

/* globals Card Encounter ActionInstance */

var Sex = new Enum
(
	"male",
	"female"
);

/* exported Rule */
class Rule
{
	constructor()
	{
		this.characterNames = [];
		this.characterPortraits = [];
		this.starterCards = [];
		this.creatureGroups = [];
		this.defaultActions = [];
	}

	generateCharacters(world, num)
	{
		let generatedChars = [];
		let excludingNames = [];
		let excludingPortraits = [];

		for (let i = 0; i < num ; i++)
		{
			let char = this.generateCharacter(world, excludingNames, excludingPortraits);
			if (char)
			{
				generatedChars.push(char);
				excludingNames.push(char.displayName);
				excludingPortraits.push(char.smallPortrait);
			}
		}

		return generatedChars;
	}

	generateCharacter(world, excludingNames, excludingPortraits)
	{
		let sex = Enum.randomValue(Sex);
		let names = [];
		let portraits = [];

		for (let nameDef of this.characterNames)
		{
			if (nameDef.sex === sex)
			{
				names = names.concat(nameDef.names);
			}
		}

		for (let portraitDef of this.characterPortraits)
		{
			if (portraitDef.sex === sex)
			{
				portraits = portraits.concat(portraitDef.portraits);
			}
		}

		if (excludingNames)
		{
			for (let name of excludingNames)
			{
				let idx = names.indexOf(name);
				if (idx >= 0) names.splice(idx, 1);
			}
		}

		if (excludingPortraits)
		{
			for (let portrait of excludingPortraits)
			{
				let idx = portraits.indexOf(portrait);
				if (idx >= 0) portraits.splice(idx, 1);
			}
		}

		let name = MathEx.randomItem(names);
		let portrait = MathEx.randomItem(portraits);

		let instName = "char_" + name.replace(' ',  '_');
		let generatedChar = world.spawn("hero.base_hero", instName);

		generatedChar.displayName = name;
		generatedChar.smallPortrait = portrait;
		return generatedChar;
	}

	generateStarterDeck(world)
	{
		let deckCards = [];
		for (let cardTemplate of this.starterCards)
		{
			deckCards.push(world.spawn(cardTemplate));
		}
		return deckCards;
	}

	generateEncounter(world)
	{
		let encounter = world.spawn(Encounter);	

		let descHostile = Archive.getDef("desc.hostile");

		let group = MathEx.randomItem(this.creatureGroups);
		if (group)
		{
			let num = MathEx.randomInt(2, 4);
			for (let i = 0; i < num; i++)
			{
				let template = MathEx.randomItem(group.creatureTemplates);
				let creature = world.spawn(template);
				creature.addDescriptor(descHostile);
				encounter.entities.push(creature);
			}
		}

		return encounter;
	}

	populateDefaultActions(world, instigator, target)
	{
		let actions = [];
		for (let actionDef of this.defaultActions)
		{
			if (actionDef.canPerform(world, instigator, target))
			{
				actions.push(new ActionInstance(actionDef, world, instigator, target));
			}
		}
		return actions;
	}
}

/* exported CharacterNameDefinition */
class CharacterNameDefinition
{
	constructor()
	{
		this.sex = Sex.male;
		this.names = [];
	}	
}

/* exported CharacterPortraitDefinition */
class CharacterPortraitDefinition
{
	constructor()
	{
		this.sex = Sex.male;
		this.portraits = [];
	}
}

/* exported CreatureGroupDefinition */
class CreatureGroupDefinition
{
	constructor()
	{
		this.creatureTemplates = [];
	}
}

