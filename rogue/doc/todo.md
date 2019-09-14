
## engine

### presenter
- [x] multiple text nodes (requires layout??)
- [ ] buttons
  - [ ] scope, global elment
    - [x] scope traversal, create global element
    - [ ] frame element change sibling offset to tree offset
  - [x] impl button, add function to check pressed() released()?, clicked()
  - [ ] input
    - [x] input process
    - [x] worker mouse interaction cache
    - [x] fix window input callback from glfw, also add enums for keys and mouse buttons
    - [x] viewport raycast function
    - [x] raycast triangle function
    - [ ] recheck hover after present() in step_frame()
- [ ] text wrap?
- [ ] more layout options (padding, margin), horizontal stack
- [ ] border
- [ ] image

### core
- [ ] make Id an actual type so it can't be used directly as index
- [ ] string store should probably be immutible too, so copy that into normal string or string attribute would work fine
  - we need be careful since if we have frequent changing strings that will cause the main allocator to expand quickly and will require reclaiming unused memory more often



## game