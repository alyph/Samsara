
### Design considerations


#### represented goods
- not all goods in real life are represented
- only goods that have economic or cultural significance will be represented

#### goods of same category

Multiple goods can be from one category and satisfy a general need
- some require all of these goods to be present to satisfy the demand (e.g. to build a warship you need wood, cloth and cannons)
- some require a number of these goods (just a good variety, e.g. food, the more variety you have the better it becomes satisfied)


#### luxury goods

The demands that come from population's standard of living may split those goods into tiers based on their prices
- richer population will demand more pricy items
- poorer population will be satisfied by standard goods, but also strive for a little bit luxury goods when their wealth permits
- some goods when they gradually become less scarce, the price will go down and even the common population can have access to those and they will lose the luxury status
- also depending on the supply difference in each region, some luxury goods may be common in some area, especially in where it's been produced. But when commerce become cheaper, those high demand goods may become luxury even at its original location or at least very pricy
- should we have a hard divide between luxury and common goods? or just let the wealthiness of the population and supply, demand scale to auto adjust its prices thus influence the demand
- maybe all population will have this tierd needs categories, so if the common people become wealthy enough to be able to access those, then it will have the same effects
  - for clothing, beauty and household items, the luxury decorations will always invoke theer high level of satisfcation, raise the appreciation of life and material world, thus generating more culture rating or also decadence
  - for food, the more variety of food provides more balanced neutrition, thus makes people healthier, the variety of spices and seasoning makes more flavourful meals that makes people happier and also more sophisticated, generate more culture
  - having access to these are always a good thing, to help your population gain access you can increase the population wealth, make those goods more abundant or make trade easier and cheaper
  - some luxury items in some very special cases, can become common goods. e.g. if gold is so prevalent that it doesn't worth anything... (this would probably never happen in any game), but even that, you can argue the gold is still gold and they are beatiful, they will still breed sophistication

#### unique goods

Some goods produced at specific locations may become unique, e.g.
- fruits produced in tropical areas (pineapples, lychee, banana etc.)
- famous clothe makers
- high quality plate armor made in Italy
- wood that grow in certain areas that is particularly strong
- certain breed of horses

Unique goods generally sell at higher price, but also produces more benefits (but how to reflect that in game?)


#### benefits of supply satisfying demand

Tiered benefit???
- maybe each good has separate supply demand gauge
- and then one need has this tierd satisfaction benefit
  - one need may involve multiple goods


### Type of goods

#### food

- generic food (local)
  - an abstraction of many different kinds of food
  - locally produced, locally trade, never trade outside
  - produced by agricultural devs by default
  - quantity is generally small and barely satisfy local demands
  - demand will not go higher than sufficient, -health +discontent if scarce
  - considered as basic food, and if grain is not available then this fills in the needs
  - but can never feel the needs above sufficient
  - when technology and culture evolves, what is considered as standard food may change
- grain
  - is basic food
  - can consider more than just actual grain, maybe some non-perishable produces and beans etc. 
  - tradable
  - can fill the needs above sufficient to plentiful and abundant
- produces (vegetables, beans, carrots, turnips, potatos etc.)
  - probably just covered in grain, non-tradable and perishable ones will just be covered by the genric local food
  - when technology advances, some more produces and fresh vegetables can be preserved and at that time, maybe a new type of goods can emerge (e.g. groccery)
- cheap alcohol (e.g. mead, beer)
  - should probably just be covered by basic food and having no significance as most of those are produced through grain anyway
- meat
  - maybe just covered by livestock
- dairy
  - maybe also covered by livestock?
  - but some of those are also non-perishable and tradable (not sure, just not included for now)
- livestock (this will be converted to meat, dairy, wool, leather etc. but also used for production)
  - major meat source
  - tradable
  - +health when fullfilled
  - some other material may also be produced from livestock like leather wool etc. but those are covered in separate goods and may just be produced along side of the livestock
- seafood
  - also meat source
  - +health
- fruits
  - secondary food
  - +health +culture
  - tradable but pretty uncommon before refrigeration is a thing
- seasoning (salt, sugar, spice)
  - each seasoning will add some culture and satisfaction
- other drinks (wine, tea, coffee)
  - each +health +culture +satisfaction +decadence

#### clothing

- generic clothing (local)
  - locally produced basic clothing
  - not traded
  - TODO: maybe just not have it represented then, not much significance, rarely there's shortage of clothing or any benefit having oversupply of clothing, the higher tier clothing though does have bigger economic impact
- textile
  - produced from wool, cotton and dye
  - +comfort +culture
- leather
  - generally side product of livestock and forest?
  - +comfort
  - reduce the demand of normal textile??
- luxury clothing (silk, fur, jewelry)
  - each +happiness +culture +decadence


#### household

- generic household items (local)
  - not tradable
  - TODO: may not be covered due to insignificance
  - local industry and agricultural devs supposed to produce those and if not, they may just produce the abstract "labor" service by default, which is generally in demand especially when the city grows bigger
- pottery
  - +quality of life (or should it just be comfort?)
  - may or may not be represented
- metal works (iron, bronze etc.)
  - +quality of life, culture
- luxury furnitures
  - produced from high quality wood with precious metals and jewelry
  - +culture +decadence
- porcelain
  - +culture +decadence
- golden, silver wares/works
  - +culture +decadence
  - should it just be golden and silver as the labor that's used to produce those are miniscule compared to the material price
  - there may be some of those unique artwork pieces that's more valuable than material itself though


#### construction materials

- basic material (wood, clay, stone)
  - generally forageable natural resource around the city 
  - used to grow the devs as well maintaining them
  - may not be necessary to represent them as goods
  - there is culture signficance as what material each culture use to build their buildings, which is influenced by where they originate (e.g. wood is most common, stone common in hilly area, clay is more common in certain area near rivers)

- marble
  - culture significance (certain culture cherish such material and use it as decorative building materials)
  - mined resource
  - used to build certain higher class devs (mostly urban)
  - the more marble accessed, the more culture those devs will produce (as a buff?)
  - the shifting in accessible tier is slow as those marbles go away slowly, so a shortage of marble will take a long time to make impact


#### industrial goods

Each of these types of goods are needed for specific types of work

- tools / machinary
  - for industrial, agricultural production, building constructions etc.
- metal (e.g. iron, copper, tin etc.)
  - raw material for producing other goods (e.g. tools, weapons), constructions of buildings etc.
- ships
  - for commerce and war
- weapons
  - for war
  - swords, blades, spears, bows
  - armor
  - guns
  - cannons
- horse
  - for commerce, war, transportations as well as industrial, algricultral production
- paper
  - for education, artwork, religious practices, administration, commerce etc.
- incense
  - religious practices (maybe also household)
  - although this is a prime example of the design thinking that not all resource in the human history needs to be covered, it doesn't need to exist in the game if it doesn't have big impact on the economic activities in our simulated world



#### some of the future goods

and how are we going to to incorperate them while technology progresses

- automobiles
- trains, planes
- cement
- steel
- modern weapons (guns, tanks, jets etc.)
- electronics
- modern appliances
- electricty and power
- clean water (this may be the part of the standard city infrustructures, same as sewage, power, internet)




