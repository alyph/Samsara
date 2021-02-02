
#pragma once

#include <vector>
#include "id.h"
#include "attribute.h"
#include "math_types.h"
#include "color.h"
#include "buffer.h"
#include "macros.h"
#include "string.h"
#include "types.h"
#include "input.h"
#include "bitset.h"
#include <functional>

struct Frame;
struct Context;
struct ScopedChildrenBlock;
struct PresentWorker;
struct Element;
struct ElementType;
struct ElementTypeSetup;
struct RaycastResult;

using FinalizerFunc = void(*)(Frame& frame, Id root_elem_id, Id first_elem_id, Id last_elem_id);
// x, y in screen space (in pixels)
using RaycasterFunc = RaycastResult(*)(const Frame& frame, Id elem_id, double x, double y);
using RendererFunc = void(*)(const Frame& frame, Id elem_id);
using ElemTypeInitFunc = std::function<void(ElementTypeSetup&)>;

enum class ElementPlacement
{
	structured,
	loose,
};

enum class MouseInteraction
{
	left_down,
	right_down,
	mid_down,
	hover,
	max
};

namespace attrs
{
	extern Attribute<FinalizerFunc> finalizer;
	extern Attribute<RaycasterFunc> raycaster;
	extern Attribute<RendererFunc> renderer;
	extern Attribute<ElementPlacement> placement;
	extern Attribute<Scalar> top;
	extern Attribute<Scalar> bottom;
	extern Attribute<Scalar> left;
	extern Attribute<Scalar> right;
	extern Attribute<Scalar> width;
	extern Attribute<Scalar> height;
	extern Attribute<Mat44> transform;
	extern Attribute<Color> background_color;
	extern Attribute<Color> foreground_color;
	extern Attribute<Id> texture;
	extern Attribute<Id> shader;
	extern Attribute<String> text;
}

extern Id register_elem_type(ElemTypeInitFunc init_func);
//template<typename T> void set_elem_type_attr(Id type_id, const Attribute<T>& attr, const T& value);

extern Id make_element(const Context& context, Id type_id);
inline const Element& get_element(const Frame& frame, Id elem_id);
extern Id get_parent(const Frame& frame, Id elem_id);
extern Id get_first_child(const Frame& frame, Id elem_id);
inline Id get_last_child(const Frame& frame, Id elem_id);
extern Id get_next_sibling(const Frame& frame, Id elem_id);
inline Id get_last_in_subtree(const Frame& frame, Id root_elem_id); // TODO: rename to get_last_descendant()??
inline size_t num_elems_in_subtree(const Frame& frame, Id root_elem_id);

template<typename T> const T* get_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T& get_elem_attr_or_default(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T& get_elem_attr_or_assert(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T* get_elem_defined_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> T& get_mutable_elem_attr_or_assert(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T, typename ValT> void set_elem_instance_attr(const Context& context, const Attribute<T>& attr, const ValT& val);
template<typename T, typename ValT> void set_elem_post_attr(Frame& frame, Id elem_id, const Attribute<T>& attr, const ValT& val);

// TODO: can maybe inline these context functions
extern Context create_scoped_context(const Context& parent_scope_context, uint64_t count);
extern Context create_scoped_context(const Context& parent_scope_context, uint64_t count, uint64_t user_id);
extern Id get_working_elem_id(const Context& context);
extern Element& get_working_elem(const Context& context);
extern void finalize(const Context& context, bool open_tree=false); // run finalize (attributes) on all unfinalized elements
extern Id get_section_root(const Context& context);

// time
inline double current_frame_time(const Context& context);

// input related
extern bool is_elem_hover(const Context& context);
extern bool is_elem_down(const Context& context, MouseInteraction interaction);
extern bool was_elem_clicked(const Context& context);
extern bool was_elem_pressed(const Context& context);
inline bool was_key_pressed(const Context& context, Keys key, ModKeys mod = ModKeys::none);
inline bool is_mouse_down(const Context& context, MouseButtons button);
inline int mouse_wheel_delta(const Context& context);
inline uint64_t received_char_code(const Context& context);

// focused elements
inline bool is_focused(const Context& context);
inline bool was_focused(const Context& context);
inline bool will_lose_focus(const Context& context);
inline void gain_focus(const Context& context);
inline void lose_focus(const Context& context);

// states
enum class ElementStateMode: uint8_t
{
	normal,
	focused,
};

template<typename T> T& access_elem_state(const Frame& frame, Id elem_id, ElementStateMode mode);
extern void* access_elem_state_buffer(const Frame& frame, Id elem_id, Id state_type_id, size_t state_size, ElementStateMode mode, bool& out_new);
extern Id new_state_type_id();

#define _ctx create_scoped_context(ctx, __COUNTER__)
#define _ctx_id(user_id) create_scoped_context(ctx, __COUNTER__, user_id)
#define _attr(attr, val) set_elem_instance_attr(ctx, attr, val)
#define _children if (const ScopedChildrenBlock CONCAT(children_block_, __COUNTER__){ctx}; true)
#define _frame_time (current_frame_time(ctx))
#define _hover (is_elem_hover(ctx))
#define _down (is_elem_down(ctx, MouseInteraction::left_down))
#define _right_down (is_elem_down(ctx, MouseInteraction::right_down))
#define _clicked (was_elem_clicked(ctx))
#define _pressed (was_elem_pressed(ctx))
#define _mouse_wheel_delta (mouse_wheel_delta(ctx))
#define _char_code (received_char_code(ctx))
#define _key_pressed(key, ...) (was_key_pressed(ctx, key, __VA_ARGS__))
#define _is_focused (is_focused(ctx))
#define _was_focused (was_focused(ctx))
#define _will_lose_focused (will_lose_focus(ctx))
#define _gain_focus() (gain_focus(ctx))
#define _lose_focus() (lose_focus(ctx))
#define _focused_elem_state(state_type) (access_elem_state<state_type>(*ctx.frame, get_working_elem_id(ctx), ElementStateMode::focused))

struct ScopeEntry
{
	Id local_id{};
	Id elem_guid{};
	uint32_t next_offset{};
	uint16_t depth{};
};

struct GlobalElement
{
	uint32_t parent{};
	uint32_t state_ptr{}; // addresses of state data should not exceed 32bit spaces
	//uint16_t depth{};
};

struct PresentGlobals
{
	std::vector<ElementType> elem_types;
	AttrTable elem_type_attr_table;

	// TODO: technically these do not belong to the global of everything
	// should be actually tied to a present function, each different present
	// function may create an entirely differenet set of scopes and global elements
	std::vector<ScopeEntry> scopes;
	std::vector<GlobalElement> global_elems;
	Buffer states_buffer;
};

struct Element
{
	Id guid{};
	uint16_t type{};
	uint16_t depth{};
	uint32_t tree_offset{}; // offset to the first element not in this element's sub tree 
							// (can be sibling or one of the ancestors' sibling)
							// equivalent to the number of elements in this sub tree
							// TODO: just rename it to the num_tree_elems?
	AttrListHandle inst_attrs;
	AttrListHandle post_attrs;
};

struct ElementType
{
	Id id{};
	std::string name;
	AttrListHandle type_attrs;
	bool as_section_root{};
};

struct ElementTypeSetup
{
	ElementType* type{};
	PresentGlobals* globals{};

	inline void set_name(const char* name) { type->name = name; }
	inline void as_section_root() { type->as_section_root = true; }
	template<typename T> inline void set_attr(const Attribute<T>& attr, const T& value);
};

struct RaycastResult
{
	Id hit_elem_id{};
	Vec3 point; // in NDC space (-1, 1), smaller z means closer
	Vec2 uv; // contextual 2d data (always 0 ~ 1)
	Vec2 ruv; // contextual 2d real number data (may be any value that makes sense)
	Vec2i iuv; // contextual 2d integer data
};


// Context is the unique piece of data passed down into each of the present, element creation functions
// generally speaking, each function should get its own context, by declaring "const Context" in the parameter list
// this way the caller must pass in a fresh new context, since Context is non-copyable, and with this
// all calls (originated from the entry present()) will operate within a unique context, which represents
// a unique scope, which can then be used to uniquely identify the elements created within it.

// Some function however may take a "const Context&" (reference type), which is a measure of optimization to avoid
// creating excessive amount of unnecessary extra scopes. However this form opens up the possiblity of passing
// in used copy of context from other calls, and may violate a few rules that enforces the uniqueness of the scope
// and elements.

// Thus the basic rule of thumb when passing context into such functions is as follows:
// 1. if you don't know what you are doing, always pass in "_ctx" which creates a new copy, this always works even when the function takes references
// 2. if you decide to pass in an existing context (e.g. "ctx" without the leading underscore), you forfeit the right of using the ctx in any other way
//    in your own function, you cannot use it to create any element, and you can not even use _ctx anymore, since that creates a new context
//    based on the current ctx. which means the callee now owns the ctx, and current function can no longer perform any presentation related
//    functionality other than interacting with the callee.
// 3. make_element() is an exception for rule 2, since that function is at the leaf and all it does it creating an element in the scope signified
//	  by the context. you can continue using _ctx to create more contexts, scopes and child elements after that, however you should not pass ctx
//	  to other functions that takes references and you definitely should call make_element() twice with the same ctx
// TODO: it would be great if we can enforce rule 2 somehow at the code level to emit an compile error, but probably not easy
struct Context
{
	Frame* frame{};
	PresentWorker* worker{};
	size_t scope_idx{};
	Id begin_elem_id{}; // any elements created within this context or any sub context will have id >= begin_elem_id
						// however, that is not to say there must be an element created within this context that's after this id

	Context() = default;
	Context(const Context& other) = delete;
	Context(Context&& other) = default;
	Context& operator=(const Context& other) = delete;
};

struct InputState
{
	double mouse_x{}, mouse_y{};
	int64_t mouse_wheel_pos{};
	uint64_t char_code{};
	Id mouse_interact_elems[(size_t)MouseInteraction::max]{}; //guid
	uint64_t key_button_down[4]{}; // merged key or mouse button down state (mouse occupy highest byte)
	RaycastResult mouse_hit;

	// TODO: click & double click
};

struct Frame
{
	Id frame_id{};
	Id prev_focused_elem{};
	Id curr_focused_elem{};
	Id next_focused_elem{};
	PresentGlobals* globals{};
	std::vector<Element> elements;
	Bitset presented;
	AttrTable inst_attr_table;
	AttrTable post_attr_table;
	InputState curr_input;
	InputState prev_input;
	double curr_time{};
	double prev_time{};
};

struct ScopedChildrenBlock
{
	ScopedChildrenBlock(const Context& context);
	~ScopedChildrenBlock();

private:
	PresentWorker* worker{};
	Frame* frame{};
};

struct ElementStateHeader
{
	uint32_t size;
	uint16_t type_id;
	ElementStateMode mode;
};
static inline constexpr const size_t state_header_size = Buffer::get_next_aligned(sizeof(ElementStateHeader)); //((sizeof(ElementStateHeader) + alignof(std::max_align_t) - 1) / alignof(std::max_align_t)) * alignof(std::max_align_t);


class Presenter final
{
public:
	using PresentFunc = void (*)(const Context ctx, void* param);

	Presenter();

	template<class T>
	void set_present_object(T* obj);
	void set_present_func(PresentFunc func, void* param);
	void process_control(const std::vector<InputEvent>& events);
	void step_frame(double dt);

private:
	void present();
	static void render(const Frame& frame);
	static RaycastResult raycast(const Frame& frame, double x, double y); // returned result.hit_elem_id is guid

	PresentGlobals globals;
	double time{};
	Frame curr_frame;
	InputState latest_input;
	PresentFunc present_func;
	void* present_func_param;
};

template<class T>
void obj_present_func(Context ctx, void* param)
{
	auto obj = static_cast<T*>(param);
	obj->present(ctx);
}

template<class T>
void Presenter::set_present_object(T* obj)
{
	set_present_func(obj_present_func<T>, obj);
}

template<typename T> 
inline void ElementTypeSetup::set_attr(const Attribute<T>& attr, const T& value)
{
	globals->elem_type_attr_table.set_attr(type->type_attrs, attr, value);
}

inline const Element& get_element(const Frame& frame, Id elem_id)
{
	return frame.elements[id_to_index(elem_id)];
}

inline Id get_last_child(const Frame& frame, Id elem_id)
{
	Id child_id = get_first_child(frame, elem_id);
	while (child_id)
	{
		Id next_id = get_next_sibling(frame, child_id);
		if (!next_id) { break; }
		child_id = next_id;
	}
	return child_id;
}

inline Id get_last_in_subtree(const Frame& frame, Id root_elem_id)
{
	const auto elem_idx = id_to_index(root_elem_id);
	const auto& elem = frame.elements[elem_idx];
	const auto offset_idx = (elem_idx + elem.tree_offset);
	asserts(offset_idx > elem_idx);
	return index_to_id(offset_idx - 1);
}

inline size_t num_elems_in_subtree(const Frame& frame, Id root_elem_id)
{
	const auto elem_idx = id_to_index(root_elem_id);
	const auto& elem = frame.elements[elem_idx];
	asserts(elem.tree_offset > 0);
	return elem.tree_offset;
}

template<typename T> 
const T* get_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	const auto& elem = frame.elements[id_to_index(elem_id)];
	const T* val = frame.post_attr_table.get_attr(elem.post_attrs, attr);
	return val ? val : get_elem_defined_attr(frame, elem_id, attr);
}

template<typename T> 
const T& get_elem_attr_or_default(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	const T* val = get_elem_attr(frame, elem_id, attr);
	return val ? *val : attr.default_value;
}

template<typename T> 
const T& get_elem_attr_or_assert(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	const T* val = get_elem_attr(frame, elem_id, attr);
	asserts(val);
	return *val;
}

template<typename T> 
const T* get_elem_defined_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	const auto& elem = frame.elements[id_to_index(elem_id)];
	const T* val = frame.inst_attr_table.get_attr(elem.inst_attrs, attr);
	if (val) return val;

	if (elem.type)
	{
		const auto& globals = *frame.globals;
		const auto& elem_type = globals.elem_types[id_to_index(elem.type)];
		return globals.elem_type_attr_table.get_attr(elem_type.type_attrs, attr);
	}

	return nullptr;
}

template<typename T> 
T& get_mutable_elem_attr_or_assert(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	// TODO: add static assert the attribute is mutable
	return const_cast<T&>(get_elem_attr_or_assert(frame, elem_id, attr));
}

template<typename T, typename ValT> 
void set_elem_instance_attr(const Context& context, const Attribute<T>& attr, const ValT& val)
{
	Element& elem = get_working_elem(context);
	context.frame->inst_attr_table.set_attr(elem.inst_attrs, attr, val);
}

template<typename T, typename ValT> 
void set_elem_post_attr(Frame& frame, Id elem_id, const Attribute<T>& attr, const ValT& val)
{
	Element& elem = frame.elements[id_to_index(elem_id)];
	frame.post_attr_table.set_attr(elem.post_attrs, attr, val);
}

inline double current_frame_time(const Context& context)
{
	return context.frame->curr_time;
}

inline bool test_key_down(const InputState& input, Keys key)
{
	const int idx = (int)key / 64;
	const int bit = (int)key % 64;
	uint64_t mask = (((uint64_t)1ull) << bit);
	return (input.key_button_down[idx] & mask);
}

inline bool test_mod_keys(const Context& context, ModKeys mod)
{
	if (mod == ModKeys::any)
	{
		return true;
	}

	static_assert((int)Keys::control == ((int)Keys::shift+1) && (int)Keys::alt == ((int)Keys::control+1));
	constexpr const int mod_idx = (int)Keys::shift / 64;
	constexpr const int mod_bit = (int)Keys::shift % 64;
	static_assert((int)Keys::alt / 64 == mod_idx);
	const uint64_t mod_mask = ((context.frame->curr_input.key_button_down[mod_idx] >> mod_bit) & (uint64_t)7ull);
	return (mod_mask == (uint64_t)mod);
}

inline bool was_key_pressed(const Context& context, Keys key, ModKeys mod)
{
	// TODO: if there is focused element, it should eat all the key input

	return !test_key_down(context.frame->prev_input, key) &&
		test_key_down(context.frame->curr_input, key) &&
		test_mod_keys(context, mod);
}

inline bool is_mouse_down(const Context& context, MouseButtons button)
{
	const int idx = 3;
	const int bit = 56 + (int)button;
	uint64_t mask = (((uint64_t)1ull) << bit);
	return (context.frame->curr_input.key_button_down[idx] & mask);
}

inline int mouse_wheel_delta(const Context& context)
{
	return (int)(context.frame->curr_input.mouse_wheel_pos - context.frame->prev_input.mouse_wheel_pos);
}

inline uint64_t received_char_code(const Context& context)
{
	return context.frame->curr_input.char_code;
}

inline bool is_focused(const Context& context)
{
	return get_working_elem(context).guid == context.frame->curr_focused_elem;
}

inline bool was_focused(const Context& context)
{
	return get_working_elem(context).guid == context.frame->prev_focused_elem;
}

inline bool will_lose_focus(const Context& context)
{
	const auto elem_guid = get_working_elem(context).guid;
	asserts(elem_guid == context.frame->curr_focused_elem);
	return elem_guid != context.frame->next_focused_elem;
}

inline void gain_focus(const Context& context)
{
	// TODO: we do not currently support switching focus in the same frame, if really needed, we need make sure we double buffer the focused elem state
	asserts(!context.frame->curr_focused_elem && !context.frame->next_focused_elem);
	context.frame->next_focused_elem = get_working_elem(context).guid;
}

inline void lose_focus(const Context& context)
{
	const auto elem_guid = get_working_elem(context).guid;
	asserts(elem_guid == context.frame->curr_focused_elem);
	// TODO: this means we will not have other element gain focus in the same frame, reevaulate this if we need that functionality
	asserts(context.frame->next_focused_elem == null_id || context.frame->next_focused_elem == elem_guid);
	context.frame->next_focused_elem = null_id;
}

template<typename T> 
T& access_elem_state(const Frame& frame, Id elem_id, ElementStateMode mode)
{
	bool newly_created = false;
	auto ptr = access_elem_state_buffer(frame, elem_id, T::type_id, sizeof(T), mode, newly_created);
	if (newly_created)
	{
		new(ptr) T{};
	}
	return *reinterpret_cast<T*>(ptr);
}

namespace elem
{
	// generic element in the presented tree
	inline Id node(const Context ctx)
	{
		return make_element(ctx, null_id);
	}
}
