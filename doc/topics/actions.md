

#### actions

- possible in game names:
  - deed, stratagem, endeavor, undertaking, move, manoeuvre, act, actiion activity, operation, initiative, plot, scheme, ploy, tactic, gambit, ruse
- actions take time to complete and takes up slots
  - some actions have some regular interactions (e.g. adjust marching order in a military campaign)


#### action pool

| Action | Pool | Tags | Effects |
|--------|------|------|------------|
| enact policy | pool | administration, economy |  policy rolled, enacted |
| culture evolving | pool | culture | new culture aspects rolled, emerged |
| technology achievement | pool | technology | new invention, innovation, discovery rolled, emerged |
| trade mission | single | economy | new trade partener, trade routes established |
| appoint governors etc. | pool | administration | |


#### building

- pool: available building pool based on tech, culture, religion etc.
  - there can be some unique buildings show up too based on events or as reward of other actions
- cost: 
  - realm points depending on the type (e.g. commerce building use economy points, military building use military points etc.)
  - money (state funding vs. private funding??)
    - we should rethink concept of state treasury, wealth etc. maybe it can be represented more abstractly rather than a solid cost you must pay for almost all actions
- options 
  - select a city and location (cosmetic?), there may be prerequisites for city
    - should we include the building in the pool if no city is available? (probably yes)
  - select the estate it belongs to (crown, baron, church, city etc.)?
    - is this too micro?
  - supervisor: selection based on building type, e.g. clergy used for religious building, merchant used for commerce building etc.
- effects: building is built (may take a few turns?)
  - additional effects may be rolled depending on realm and character skills. e.g. an extra mod added to the building, or temp buff for local, or reduced building time etc.


#### recruitment

- pool: available troop types depending on tech, culture, religion, locality
  - unique troops may be available as well, maybe for a limited time too
- cost:
  - military points
- options:
  - location, certain troop has prerequisites as where they can be recruited
    - each location also has limit how many troops they can support
    - most of the troop in lords' seats are recruited as lords direct retinues, whereas in other territories most will belong to local lords
  - recruiter: a general? depending on where they are too
    - wonder maybe it's too macro, maybe a global extersion of buff will work better
    - or depending on where you hire them, certain general characters buff will auto apply
- effects: troops are added to the local troop pool
  - you can only use them when mustered for the war campaign, how much you can muster depends on the war eagerness (see [military campaign](#military-campaign))

#### military campaign

- represent a military offensive
- cost:
  - military points
  - diplomacy points for certain mustering bonus?
- options
  - enemy, depending on the alliance status, may campaign against multiple enemies
  - mustering, depending on the war eagerness against the enemy, lords within the realm can be mustered to varying degrees (varied amount of troop joining, may vary on time table too). Allies may also join depending on their eagerness as well.
    - can also hire mercenaries
  - warplan, setup the warplan for this turn
    - army distribution, assign generals as commanders
    - marching path (main settlements to capture, minor settlements may be captured along the way)
- effects:
  - warplan is played out for this turn, battle may be fought, settlements may be sieged or captured
  - roll for campaign events, e.g.
    - some troops may stay, some may leave
    - ravaged country side etc.
- can call for the end of the campaign, if not, it continues next turn, until both sides are standing down
  - each turn, warplans can be adjusted, additional mustering can occur
- defender side will draw their own warplan as well, and defender can become counter-attacker if the situation goes in their favour
- medieval campaign normally won't last more than one or two years, the constant compaigning will most likely wear the lords and soldiers down, as well as their economy
- the newly captured land will usually have garrisons stationed, until the land is sucessfully absorbed (a lot of times by the new lords) or gets retaken
  - aren't all settlements garrisoned? maybe should abstract this out


#### diplomacy

- various events and actions altering geo political landscape
- cost:
  - diplomacy points
- ones to be covered
  - confederacy
  - subjugation
  - declare hegemony

##### relationship improved
- options
  - target domain
  - character may be selected as a driving force (manual or auto select?)
- effects
  - relationship score increase
  - roll for additional effects. e.g. exchange visitors, and follow up events like marriage arrangement

##### relationship deteriorated
- options
  - target domain
  - character may be selected as a driving force (manual or auto select?)
- effects
  - relationship score decrease
  - and roll for additional effects. e.g. cancel treaties etc.


##### trade agreement 
- options
  - target domain (state)
  - character leading the negotiation
- requirements
  - target domain >= min relationship
- effects
  - trade between states are protected muturally, additional benefits can be rolled e.g. more trade income

##### mutural enemy agreement
- options
  - target domain
  - enemy domain
  - character leading the scheme
- requirements
  - target domain >= min relationship
  - target domain against enemy domain has certain attitude (unfriendly, untrusted, threatened etc.)
- effects
  - domains in treaty will provide aid to each other when either goes into war against the common enemy
  - higher roll may add benefit like direct military aid in campaign

##### defensive pact
- options
  - target domain (may be multiple?)
  - character leading the negotiation
- requirements
  - target domain >= min relationship
  - bonus if target domain feel threatened
- effects
  - domains are bound together fo fend off aggressors

##### coalition
- options
  - target domains
  - sometimes a common enemy domain
  - character leading the negotiation
- requirements
  - target domain >= min relationship
  - bonus if target domain hostile against the common enemy
- effects
  - domains are bound together facing all military threats
  - sometimes they also provide economic aid to each other

##### marriage arrangement
- options
  - target domain (state?)
  - characters to be married
- requirements
  - target domain >= min relationship
- effects
  - two states will be in a temporary alliance as long as the marriage tie stands


#### action details

- **building projects**
  - building time will be determined by the scale of the project and other modifiers (e.g. size of the city)
  - can be physical buildings (usually monumental): palace, cathedral, castles, great library, foundry etc.
  - can be infrastructures: roads, walls, sewer system, irrigation, dams
  - can be some kind of development: farms, mines, pastures, plantations, vineyard, crafting district, weapon manufactory, textile workshops, arenal (ship building), center of trade, port
    - some of the development can just be opportunity (e.g. emerge of small cloth workshops), but most of those involves steps of investment, build up and maybe more appropriate handled as a project
  - random projects that meet prerequisites will be proposed during draft phase, prerequisites can include
    - technology level
    - city development level (e.g. require x industrial devs to build a forge)
    - culture and religion
    - resources (e.g. you need access to wool to build textile industry)
  - buildings can also be proposed as rewards to other actions, opportunities etc. and usually will be discounted and have unique and better modifiers
  - to commence, usually need pick a target city and optionally a person in charge, the person will be occupied until project is completed
  - some big projects will drain significant resources from local market during construction (e.g. a lot of stones to build pyramid)
  - probably will need limit how many ongoing building projects at same time. a city probably just have 1 projects at a time
  - projects will generally cost money to build, however there can be options to build it by private sector (e.g. church, merchants), in which case the build will be owned by whoever build it and the state will not gain as much benefit
- **building army**
  - recruiting and training for persisten troops, retinues (different compared to call to arms which may also build up the army, but mostly temporary)
  - proposal will show the type of the troop, and must meet prerequisites:
    - techonology, culture, resource etc.
  - building troops will cost money and can optionally assign a general to improve the quality of the recruits
  - a target city also need to be selected to decide where to recruit from, the city must meet the requirements, sometimes there may only be one city that meet the requirements
  - for tribal or feudal organizations, most troops are not owned by the state, however the pool of the local troops should still be represented (to see their types, quality and size) and retruited, upgraded based on culture and techonology, there may not be actions to influence them directly however. Although the small portion that's owned by the state can still be manually built and recruited (and those are usually the elites)
- **military campaign**
  - ongoing war effort with active military activities (simply put: with a marching army)
  - usually with clear enemies and relatively clear goal (e.g. defeat enemy armies, subjugate enemy states, capture captitals or state head)
  - there can be multiple campaigns going on at the same time, if those are for different fronts, against different enemies
  - campaigns can be self instigated (normally as attacker) or forced into (normally as defender)
  - there can be a planning phase that leads into active campaign
  - there may be a aftermath phase where result is determined, but takes some time to complete the final goal (e.g. subjugation)
  - in medieval time where standing armies are scarce, vassels and levies must be called upon to raise the army and rally to the battlefield. usually a rallying point is set and it takes some time for armies to assemble
  - there may also be several diplomatic effort to call upon allies or prearanged helps from other states to bring their armies together to form a bigger army.
  - there will also be options to bring in mercenaries
  - in medieval time, usually one or two big stack of armies are formed, and marching in unison. But usually there will be detachment and garrison to take control over territories along the marching path, those usually are taken care of automatically and can think of some portions of the army are not always with the main host, and ultimately the contact with enemy armies will lead to a few (a lot of times, one) decisive battles that decide the fate of the campaign, the winner can usually quickly take control of the rest of the crucial land and the loser usually will not have enough time to reorganize a second host. Although nothing is always the same, sometimes the defender may be able to hold out in fortress and wait out the winter to fend off the besieger or even find ways to call up (or buy) another army to counter attack. A lot of times, the winner also is under the press of finishing campaign quickly, so usually it will lead into negotiation or partial occupying the land or truce
  - sometimes with the death of the ruler/commander the campaign can end quicky (e.g. Hasting), also for war goals like claiming throne, with one side ruler died, the other side automatially gain the legitimacy for the throne and without other contenders the war can end easily with winner on the throne
  - campaigns and battles have a severe impact on the economy
    - wars between states disrupt the trade (any trade passing by the involved states may be hindered)
    - campaign and march creates a path of destruction (looting, pillaging are common in middle ages, armies also sometimes feed on the land by foraging)
    - campaigning drains the coffer
    - large portion of the armies are drafted and levied from peasants, and the casualties of those greatly reduces the work force. 
    - casualties on nobel warriors and their retinues are also expensive to replace
- **diplomatic missions**
  - these actions are mostly for dealing with negotiations and relationships with other states or sometimes with vassels in the realm
  - normally these missions have clear goals and last for a period of time but not persist indefinitely
  - missions can inlcude:
    - improve diplomatic relationship, usually involves a few visits, exchange of gifts and potentially setting up more regular ties or even political marriage, which will serve as a persistent relationship boost
    - open trade agreement
    - different pacts, alliances and agreements
    - negotiations, peace and truce talk (often followed by a military campaign), there's also third party peace making effort
    - betrothal and wedding arrangement
    - diplomatic subjugation (vasselization)
    - covert actions like support dissidents, revolts, independence and pretenders (some of these are more like intrigues), also sow dissent
    - build spy network?
    - recruite allies or borrow armies for a common enemy, incite one state aggression against another state (sometimes can be the state's own people or vassels), these can also be well coordinated with war preparation for military campaign
    - a follow up of above, also searching for potential allies both inside the enemy and outside (e.g. neighbors)
    - religious missions, e.g. seeking support from pope or even make your own anti-pope
  - a renowned statesman can be assigned to the mission, as well as members of ruling family
  - some of these actions can be initiated from certain opportunities (e.g. visit of a pretender or title claimer seeking support)
- **trade missions**
  - usually the trade missions are conducted by merchants (invested by private sectors) instead of by the state
  - but sometimes there's also missions sanctioned by the ruler directly and with certain level of state's military and diplomatic support
  - missions include (incomplete list):
    - create new trade route
    - increase trade of certain goods
    - direct trade of certain goods
    - monopolize certain trade
    - creating trade center and improve trading infrustructure (probably will be covered by building actions)
  - if you have renowned merchant characters, they can be used to conduct those missions and with extra efficiencies and sometimes more options
  - note that even if the missions are conducted by private sector, the player can still issue and control such actions, because remember the player doesn't just represent the state, it represent the whole realm
- intrigue
- quests and mission
  - pilgrimage
  - crusade
- social and economic development and reform
  - enact laws
  - enact national policies
  - enact local edicts



#### opportunities

- opportunties are like stroke of genius. it doesn't take time to complete and will take effects right away.
  - it will generally cost some idea points
  - some will actually spawn an action that will still take slots???


- new technological innovations
- new cultural ideas
- new national spirits and ideas
  - a ruler's ambition may spark a fledgeling idea, and this idea may grow and mature over time, through the works of the rulers as well as many around him or even by the entire nation, so such opportunity may appear time and time again, and grow stronger each time. there may be actions that can take to further develop said ideas



#### agendas

- agendas are long term goals and ambitions
  - usually there will be one agenda originated from the ruler's ambition
  - agendas will last until fullfilled and in the meantime, it will provide specific actions to help with the course
  - agendas can also cease due to certain events, e.g. ruler's death, change of mind


