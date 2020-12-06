## Board

https://github.com/alyph/Samsara/projects/1

## engine

### presenter
- [X] ~~*multiple text nodes (requires layout??)*~~ [2020-11-25]
- [X] ~~*buttons*~~ [2020-11-25]
  - [X] ~~*scope, global elment*~~ [2020-11-25]
    - [X] ~~*scope traversal, create global element*~~ [2020-11-25]
    - [X] ~~*frame element change sibling offset to tree offset*~~ [2020-11-25]
  - [X] ~~*impl button, add function to check pressed() released()?, clicked()*~~ [2020-11-25]
    - [X] ~~*do click state change*~~ [2020-11-25]
  - [X] ~~*input*~~ [2020-11-25]
    - [X] ~~*input process*~~ [2020-11-25]
    - [X] ~~*worker mouse interaction cache*~~ [2020-11-25]
    - [X] ~~*fix window input callback from glfw, also add enums for keys and mouse buttons*~~ [2020-11-25]
    - [X] ~~*viewport raycast function*~~ [2020-11-25]
    - [X] ~~*raycast triangle function*~~ [2020-11-25]
    - [X] ~~*recheck hover after present() in step_frame()*~~ [2020-11-25]
- [X] ~~*forward layouting*~~ [2020-11-25]
  - [X] ~~*finalizer processing*~~ [2020-11-25]
    - finalizer to fill in derived attributes (e.g. layout)
    - finalizer also collects renderable buffers for common elements (e.g. text, image, colored boxes)
  - [X] ~~*allow elements to find its section root*~~ [2020-11-25]
    - so it can access the shared renderable buffer and add custom buffer data to it
  - [X] ~~*impl tablet finalizer with layout and render buffer*~~ [2020-11-25]
- [X] ~~*text should influence width if not determine it (perhaps should determine max width?),*~~ [2020-11-25] 
  - one work around for now is to let the high level function (like button()) preset width based on the length of the text
- [ ] accurate mouse button and key down states (even if mouse leave the window or app lost focus)
- [ ] a potential optimization: only perform rendering related code (like inserting glyphs to the buffer) when we actualy performing the final present before rendering rather than for all the steps including process input, may require the presenter passing a flag (probably thru variable in the context)
- [ ] text wrap?
- [ ] more layout options (padding, margin), horizontal stack
- [ ] border
- [ ] image
- [ ] find a way to prevent _ctx passing into const context& function which supposed to receive ctx...

### tablet
- [ ] cache tablet based on guid
- [ ] tablet element post rendering, so we can render decoration stuff like borders and selection highlights etc.

### core
- [ ] refactor array
  - [X] ~~*perform construction*~~ [2020-11-25]
  - [ ] perform copy
  - [ ] provide a func for trivial copy
  - [X] ~~*remove constant checking of alloc handle, just access the pointer*~~ [2020-11-25]
  - [X] ~~*allow alloc array in specified allocator*~~ [2020-11-25]
  - [ ] arrayview should be just like stringview, let it implicitly construct from array
- [ ] refactor string
  - [X] ~~*remove ref counting, deallocation, string header*~~ [2020-11-25]
  - [ ] allow storing string in specified allocator
  - [X] ~~*do we still need stringview in that case? (DONE: removed)*~~ [2020-11-25]
- [ ] simplify allocator
  - [X] ~~*allocate memory in page so pointer never expire*~~ [2020-11-25]
  - [X] ~~*no more deallocation*~~ [2020-11-25]
  - [X] ~~*alloc id only used for when allocator bulk release*~~ [2020-11-25]
  - [ ] rethink shared array
  - [X] ~~*rethink reallocation and move allocation*~~ [2020-11-25]
- [ ] bucket array? (maybe call it paged array)
  - [ ] pooled bucket array, has a main pool and each array will get allocated buckets from that pool, and they will have access to a control block from the pool
- [ ] make Id an actual type so it can't be used directly as index
- [ ] replace all std::string with String
- [ ] replace all std::vector with Array
- [ ] finish impl simple array
- [ ] attribute should have a default constructor with T{}
- [X] ~~*string store should probably be immutible too, so copy that into normal string or string attribute would work fine*~~ [2020-11-25]
  - we need be careful since if we have frequent changing strings that will cause the main allocator to expand quickly and will require reclaiming unused memory more often
- [X] ~~*string builder, string format*~~ [2020-11-25]
- [X] ~~*perf: add a string layout which points directly to the pointer of a string literal*~~ [2020-11-25]
- [ ] have custom allocator (declared as static variable and auto assign an allocator id, limited number 16 right now), the custom allocator should also be configurable (e.g. temp or not, reclaimation strategy)
- [ ] better and simpler engine initialization, call a set of functions to perform all necessary init, rather than fully rely on the constructor init

### allocator
- [ ] handle Allocator::none, maybe we should get idx = (allocator - 1)
- [X] ~~*allow allocate 0 bytes data, which will return an empty handle but record the allocator and later can use reallocate to get real memory*~~ [2020-11-25]
- [ ] contextual perm allocator (no stack, always scoped, one time use)

### input
- [X] ~~*mouse wheel event*~~ [2020-11-25]
- [ ] key input with mods (shift, ctrl etc.)



## editor
- [ ] basic map painting
  - [ ] brushes
    - [X] ~~*square brush*~~ [2020-11-25]
    - [ ] round bursh
    - [X] ~~*brush size*~~ [2020-11-25]
  - [X] ~~*save/load map*~~ [2020-11-25]
  - [ ] tile types
    - [X] ~~*coast*~~ [2020-11-25]
    - [ ] ocean
    - [ ] river
  - [X] ~~*map runtime visual data (these data are recreated when map loaded or new map created)*~~ [2020-11-25]
    - [X] ~~*map glyph cache*~~ [2020-11-25]
    - [X] ~~*update glyph when changing the tile terrain and structure*~~ [2020-11-25]
  - [ ] city placement
    - [X] ~~*city data (and devs)*~~ [2020-11-25]
    - [X] ~~*button to create city (comes with intial set of devs)*~~ [2020-11-25]
    - [X] ~~*city wall (first pass, just rect wall bounding box encompassing all urban devs)*~~ [2020-11-25]
    - [X] ~~*select city*~~ [2020-11-25]
    - [X] ~~*city expansion*~~ [2020-11-25]
    - [X] ~~*rural devs (simple placement outside of the city)*~~ [2020-11-25]
    - [X] ~~*houses (some of these will be replaced by template or auto gen)*~~ [2020-11-25]
    - [ ] road connection (connects cities)
  - [ ] features and buildings
    - [ ] resource sites
    - [ ] building placement
  - [ ] map view
    - [X] ~~*map panning*~~ [2020-11-25]
    - [X] ~~*map zoom*~~ [2020-11-25]
    - [X] ~~*reference map*~~ [2020-11-25]
    - [ ] multi-stage zoom
  - [ ] toggle between game/editor
  - [ ] toggle ref images
  - [ ] multiple ref images and move and scale
  - [ ] add a save/load button

## game
- [ ] turn/time tracking
- [ ] turn actions
  - play cards?
  - what other actions if no card to play?? (draw extra cards? discard and reshuffle?)
- [ ] realm points
- [ ] event card system
  - [ ] cycling and drawing of the cards
  - [ ] play sequence (choose target etc.)
  - [ ] event effects
- [ ] economic model
  - [ ] development
  - [ ] production
  - [ ] trade
- [ ] building system
- [ ] warfare
- [ ] technology
- [ ] culture
- [ ] characters
- [ ] political systems
- [ ] diplomacy

