
## engine

### presenter
- [x] multiple text nodes (requires layout??)
- [ ] buttons
  - [x] scope, global elment
    - [x] scope traversal, create global element
    - [x] frame element change sibling offset to tree offset
  - [x] impl button, add function to check pressed() released()?, clicked()
    - [ ] do click state change
  - [x] input
    - [x] input process
    - [x] worker mouse interaction cache
    - [x] fix window input callback from glfw, also add enums for keys and mouse buttons
    - [x] viewport raycast function
    - [x] raycast triangle function
    - [x] recheck hover after present() in step_frame()
- [ ] forward only layout and maybe allow deferred parent??
- [ ] text should influence width if not determine it (perhaps should determine max width?), 
  - one work around for now is to let the high level function (like button()) preset width based on the length of the text
- [ ] text wrap?
- [ ] more layout options (padding, margin), horizontal stack
- [ ] border
- [ ] image

### core
- [ ] make Id an actual type so it can't be used directly as index
- [x] string store should probably be immutible too, so copy that into normal string or string attribute would work fine
  - we need be careful since if we have frequent changing strings that will cause the main allocator to expand quickly and will require reclaiming unused memory more often
- [x] string builder, string format
- [ ] perf: add a string layout which points directly to the pointer of a string literal
- [ ] have custom allocator (declared as static variable and auto assign an allocator id, limited number 16 right now), the custom allocator should also be configurable (e.g. temp or not, reclaimation strategy)



## game