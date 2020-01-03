
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
#include <functional>

struct InputEvent;
struct Frame;
struct Context;
struct ScopedChildrenBlock;
struct PresentWorker;
struct Element;
struct ElementType;
struct ElementTypeSetup;
struct RaycastResult;

using FinalizerFunc = void(*)(Frame& frame, Id root_elem_id, Id first_elem_id, Id last_elem_id);
// x, y in screen space (in pixels), out_z in NDC space (-1, 1)
using RaycasterFunc = Id(*)(const Frame& frame, Id elem_id, double x, double y, double& out_z);
using RendererFunc = void(*)(const Frame& frame, Id elem_id);
using ElemTypeInitFunc = std::function<void(ElementTypeSetup&)>;

namespace attrs
{
	extern Attribute<FinalizerFunc> finalizer;
	extern Attribute<RaycasterFunc> raycaster;
	extern Attribute<RendererFunc> renderer;
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
	extern Attribute<StringView> text;
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
extern Element& get_working_elem(const Context& context);
extern void finalize(const Context& context, bool open_tree=false); // run finalize (attributes) on all unfinalized elements
extern Id get_section_root(const Context& context);

// input related
extern bool is_elem_hover(const Context& context);
extern bool is_elem_down(const Context& context);
extern bool was_elem_clicked(const Context& context);
extern bool was_elem_pressed(const Context& context);

#define _ctx create_scoped_context(ctx, __COUNTER__)
#define _ctx_id(user_id) create_scoped_context(ctx, __COUNTER__, user_id)
#define _attr(attr, val) set_elem_instance_attr(ctx, attr, val)
#define _children if (const ScopedChildrenBlock CONCAT(children_block_, __COUNTER__){ctx}; true)
#define _hover (is_elem_hover(ctx))
#define _down (is_elem_down(ctx))
#define _clicked (was_elem_clicked(ctx))
#define _pressed (was_elem_pressed(ctx))


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
	Vec3 point; // in ndc
	Vec2 uv; // contextual 2d data
	IVec2 iuv; // contextual 2d integer data
};

struct Context
{
	Frame* frame{};
	PresentWorker* worker{};
	size_t scope_idx{};
	Id begin_elem_id{};

	Context() = default;
	Context(const Context& other) = delete;
	Context(Context&& other) = default;
	Context& operator=(const Context& other) = delete;
};

enum class MouseInteraction
{
	left_down,
	right_down,
	mid_down,
	hover,
	max
};

struct InputState
{
	double mouse_x{}, mouse_y{};
	Id mouse_interact_elems[(size_t)MouseInteraction::max]{}; //guid
	uint64_t key_button_down[4]{}; // merged key or mouse button down state (mouse occupy highest byte)

	// TODO: click & double click
};

struct Frame
{
	Id frame_id{};
	PresentGlobals* globals{};
	std::vector<Element> elements;
	AttrTable inst_attr_table;
	AttrTable post_attr_table;
	InputState curr_input;
	InputState prev_input;
};

struct ScopedChildrenBlock
{
	ScopedChildrenBlock(const Context& context);
	~ScopedChildrenBlock();

private:
	PresentWorker* worker{};
	Frame* frame{};
};

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
	static Id raycast(const Frame& frame, double x, double y); // return guid

	PresentGlobals globals;
	double time{};
	Frame curr_frame;
	InputState latest_input;
	PresentFunc present_func;
	void* present_func_param;


	// void process_input()
	// {
	// 	// change input state -> mouse down / up, mouse hover etc.
	// 	// derived input event -> mouse click, double click, mouse enter / exit, key down, key up
	// 	present_frame(); // with the new input state and event
	// }

	// void present_frame()
	// {
	// 	new_frame(); // prepare new frame data based on previous frame data, carry over ui state, input state, event etc.
	// 	do_frame(); // call custom presenter

	// 	// analyze new input state if changed call present_frame() again (only once though)
	// }

	// void render_frame()
	// {
	// 	// draw using the current frame data
	// }
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

// template<typename T>
// void set_elem_type_attr(Id type_id, const Attribute<T>& attr, const T& value)
// {

// }

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

namespace elem
{
	// generic element in the presented tree
	inline Id node(const Context ctx)
	{
		return make_element(ctx, null_id);
	}
}
