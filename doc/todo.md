
## engine

### presenter
- [x] multiple text nodes (requires layout??)
- [x] buttons
  - [x] scope, global elment
    - [x] scope traversal, create global element
    - [x] frame element change sibling offset to tree offset
  - [x] impl button, add function to check pressed() released()?, clicked()
    - [x] do click state change
  - [x] input
    - [x] input process
    - [x] worker mouse interaction cache
    - [x] fix window input callback from glfw, also add enums for keys and mouse buttons
    - [x] viewport raycast function
    - [x] raycast triangle function
    - [x] recheck hover after present() in step_frame()
- [x] forward layouting
  - [x] finalizer processing
    - finalizer to fill in derived attributes (e.g. layout)
    - finalizer also collects renderable buffers for common elements (e.g. text, image, colored boxes)
  - [x] allow elements to find its section root
    - so it can access the shared renderable buffer and add custom buffer data to it
  - [x] impl tablet finalizer with layout and render buffer
- [x] text should influence width if not determine it (perhaps should determine max width?), 
  - one work around for now is to let the high level function (like button()) preset width based on the length of the text
- [ ] accurate mouse button and key down states (even if mouse leave the window or app lost focus)
- [ ] a potential optimization: only perform rendering related code (like inserting glyphs to the buffer) when we actualy performing the final present before rendering rather than for all the steps including process input, may require the presenter passing a flag (probably thru variable in the context)
- [ ] text wrap?
- [ ] more layout options (padding, margin), horizontal stack
- [ ] border
- [ ] image
- [ ] find a way to prevent _ctx passing into const context& function which supposed to receive ctx...

### core
- [ ] refactor array
  - [x] perform construction
  - [ ] perform copy
  - [ ] provide a func for trivial copy
  - [x] remove constant checking of alloc handle, just access the pointer
  - [x] allow alloc array in specified allocator
  - [ ] arrayview should be just like stringview, let it implicitly construct from array
- [ ] refactor string
  - [x] remove ref counting, deallocation, string header
  - [ ] allow storing string in specified allocator
  - [x] do we still need stringview in that case? (DONE: removed)
- [ ] simplify allocator
  - [x] allocate memory in page so pointer never expire
  - [x] no more deallocation
  - [x] alloc id only used for when allocator bulk release
  - [ ] rethink shared array
  - [x] rethink reallocation and move allocation
- [ ] bucket array? (maybe call it paged array)
  - [ ] pooled bucket array, has a main pool and each array will get allocated buckets from that pool, and they will have access to a control block from the pool
- [ ] make Id an actual type so it can't be used directly as index
- [ ] replace all std::string with String
- [ ] finish impl simple array
- [ ] attribute should have a default constructor with T{}
- [x] string store should probably be immutible too, so copy that into normal string or string attribute would work fine
  - we need be careful since if we have frequent changing strings that will cause the main allocator to expand quickly and will require reclaiming unused memory more often
- [x] string builder, string format
- [x] perf: add a string layout which points directly to the pointer of a string literal
- [ ] have custom allocator (declared as static variable and auto assign an allocator id, limited number 16 right now), the custom allocator should also be configurable (e.g. temp or not, reclaimation strategy)

### allocator
- [ ] handle Allocator::none, maybe we should get idx = (allocator - 1)
- [ ] allow allocate 0 bytes data, which will return an empty handle but record the allocator and later can use reallocate to get real memory

### input
- [ ] mouse wheel event



## editor
- [ ] basic map painting
  - [ ] brushes
    - [x] square brush
    - [ ] round bursh
    - [x] brush size
  - [ ] save/load map
  - [ ] tile types
    - [x] coast
    - [ ] ocean
    - [ ] river
  - [ ] map runtime visual data (these data are recreated when map loaded or new map created)
    - [ ] global map glyph table
    - [ ] terrain glyphs (index to glyph table)
    - [ ] map object glyphs
  - [ ] city placement
    - [x] city data (and devs)
    - [x] button to create city (comes with intial set of devs)
    - [x] city wall (first pass, just rect wall bounding box encompassing all urban devs)
    - [x] select city
    - [ ] city expansion
    - [ ] road connection
    - [ ] rural devs (simple placement outside of the city)
    - [ ] resource sites
    - [ ] houses (some of these will be replaced by template or auto gen)
    - [ ] road (connects cities)
  - [x] map panning
  - [ ] map zoom

## game
- [ ] economic model
  - [ ] development
  - [ ] production
  - [ ] trade
- [ ] culture
- [ ] technology
- [ ] political systems
- [ ] diplomacy
- [ ] warfare
- [ ] characters
- [ ] action mechanics

