/*exported NumericDieRoll*/
class NumericDieRoll
{
	constructor(numDice, numSides, plus)
	{
		this.numDice = Math.max(numDice || 1, 1);
		this.numSides = Math.max(numSides || 1, 1);
		this.plus = plus || 0;
	}

	roll()
	{
		let total = 0;
		for (let i = 0; i < this.numDice; i++)
		{
			total += MathEx.randomInt(1, this.numSides);
		}
		total += this.plus;
		return total;
	}
}