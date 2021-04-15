## presenter system


### elments

- elements are the basic building blocks of the presented interface
- created via the make_elemewnt() function
- elements have attributes associated with therm to describe
  - layout: positioning, sizing etc.
  - style: color, border etc.
  - behavior: input flags, focusable etc.
  - implementations: rendering, raycasting etc.
- element may also have a type, which basically comes with a set of attributes predefined
- element attributes are set AFTER elements are created
- input states are always calculated at element level and associated to each individual elemewnts


### components

- components are high level constructs that encapsualte a reusable piece of presentation and behavior
- each component usually creates and manages one or more elements
- with immediate-mode, components are in the form of functions
- components can be customized and configured via the following interface:
  - layout
  - style
  - other function parameters
- note the difference between these customizations and attributes is that here, all the configs must be defined and passed into the component functions before the components are even created, whereas attributes are set later after the elements are created
- they are however related, such that the components will assign attributes to each of their sub elements based on the parameters passed in
- some layout, styling parameters may be direct pass-through to the elements, espcially for simple one element component (e.g. node())
- input states are not associated directly to the components, however that can normally be inferred by inspecting the input state of the root element of the component (or one of the root if the component has multiple root elements). input states and other interactions are commonly returned back from the component functions and can be inspected that way by the caller. (e.g. button() returns true when clicked)


### layout and style

- these are the parameters passed to the components
- they are high level concepts but are similar to attributes (which are lower level)
- because the layout and style values may be passed to attributes, they are organzied and stored in a similar fashion as attributes: with a key and value pair
- each component can define their own set of layouting and styling parameters, and some may use a standard set of parameters (may be exactly same as attributes, e.g. width, height, background color, foreground color etc.)
- a little bit of differentiation between layout and style
  - style concerns the look (and/or sound), and usually many elments may share the same style
  - layout concerns positioning and organizations, and usually each element will have some unique layout values (although they may also share some of those, e.g. padding, wrapping, flow direction etc.)
- with that consideration, we may not have a hard distinction between layout and style, but we may allow both shared and unique layouting/styling/theming pararmeters
- shared parameters may be packaged in a theme object and pass around to be inherited by multiple components. now here we slightly altered what theme means here, theme here represent both layouting and styling parameters, but focus on the shared portion of those
- and for each component, we can also pass in a unique set of parameters
- note we may also have compound theming parameters, which are parameters that themselves are themes, for example, for a window themed parameter, there may ber button_theme, text_theme etc. in the button theme, there may be text_theme as well


### parameters defined after components creation

- normally all parameters (layouting, styling and other state params) are defined and passed into the components functions before components are even created (for this frame)
- there may be cases where we cannot determine some of the parameters before the component has been created. For example, we may need check the input status on that component to change its color.
- first we should avoid these cases whereever possible
  - one mitigation is to create a wrapper element or simple component (typically node()) as the parent of the component we trying to check input on. We check input on the wrapper component instead, and then set style for the actual component.
- if this is really necessary, we can provide a look ahead function which pre-inspects the input of the will-be components based on previous frame's scope information based on the assumption of the scope structure never changes and the input we inspect is always calculated from the previous frame output. However since we don't have the actual element created for this frame yet, all we can do is perform a DFS and find the first scope that's associated with a prior element and has a guid defined, but one danger is if the component is changing its root element creation or making more root elmemnt then this pre-inspection is off. Which is probably not a big deal and that altering root element way is not a common practice anyway.





