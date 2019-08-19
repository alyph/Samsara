
#pragma once

#include <vector>
#include "id.h"
#include "attribute.h"
#include "math_types.h"
#include "color.h"
#include "buffer.h"
#include "macros.h"
#include "string.h"
#include <functional>

struct InputEvent;
struct Frame;
struct Context;
struct ScopedChildrenBlock;
struct PresentWorker;
struct Element;
struct ElementType;
struct ElementTypeSetup;

using RendererFunc = void(*)(const Frame& frame, Id elem_id);
using PostProcessorFunc = void(*)(Frame& frame, Id elem_id);
using ElemTypeInitFunc = std::function<void(ElementTypeSetup&)>;

namespace attrs
{
	extern Attribute<RendererFunc> renderer;
	extern Attribute<PostProcessorFunc> postprocessor;
	extern Attribute<double> top;
	extern Attribute<double> bottom;
	extern Attribute<double> left;
	extern Attribute<double> right;
	extern Attribute<double> width;
	extern Attribute<double> height;
	extern Attribute<Mat44> transform;
	extern Attribute<Color> background_color;
	extern Attribute<Id> texture;
	extern Attribute<Id> shader;
	extern Attribute<String> text;
}

extern Id register_elem_type(ElemTypeInitFunc init_func);
//template<typename T> void set_elem_type_attr(Id type_id, const Attribute<T>& attr, const T& value);

extern Id make_element(const Context& context, Id type_id);
extern Id get_first_child(const Frame& frame, Id elem_id);
extern Id get_next_sibling(const Frame& frame, Id elem_id);
inline Id get_last_in_subtree(const Frame& frame, Id root_elem_id); // TODO: rename to get_last_descendant()??

template<typename T> const T* get_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T& get_elem_attr_or_default(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T& get_elem_attr_or_assert(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T* get_elem_defined_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T, typename ValT> void set_elem_instance_attr(const Context& context, const Attribute<T>& attr, const ValT& val);
template<typename T, typename ValT> void set_elem_post_attr(Frame& frame, Id elem_id, const Attribute<T>& attr, const ValT& val);

// TODO: can maybe inline these context functions
extern Context create_scoped_context(const Context& parent_scope_context, uint64_t count);
extern Context create_scoped_context(const Context& parent_scope_context, uint64_t count, uint64_t user_id);
extern Element& get_working_elem(const Context& context);

#define _ctx create_scoped_context(ctx, __COUNTER__)
#define _ctx_id(user_id) create_scoped_context(ctx, __COUNTER__, user_id)
#define _attr(attr, val) set_elem_instance_attr(ctx, attr, val)
#define _children if (const ScopedChildrenBlock CONCAT(children_block_, __COUNTER__){ctx}; true)

struct PresentGlobals
{
	std::vector<ElementType> elem_types;
	AttrTable elem_type_attr_table;
};

struct Element
{
	uint16_t type{};
	uint16_t depth{};
	uint32_t sibling_offset{};
	AttrListHandle inst_attrs;
	AttrListHandle post_attrs;
};

struct ElementType
{
	Id id{};
	std::string name;
	AttrListHandle type_attrs;
};

struct ElementTypeSetup
{
	ElementType* type{};
	PresentGlobals* globals{};

	void set_name(const char* name);
	template<typename T> void set_attr(const Attribute<T>& attr, const T& value);
};

struct Context
{
	Frame* frame{};
	PresentWorker* worker{};
	Id scope{};

	Context() = default;
	Context(const Context& other) = delete;
	Context(Context&& other) = default;
	Context& operator=(const Context& other) = delete;
};

struct Frame
{
	Id frame_id{};
	const PresentGlobals* globals{};
	std::vector<Element> elements;
	AttrTable inst_attr_table;
	AttrTable post_attr_table;
};

struct ScopedChildrenBlock
{
	ScopedChildrenBlock(const Context& context);
	~ScopedChildrenBlock();

private:
	PresentWorker* worker{};
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

	PresentGlobals globals;
	double time{};
	Frame curr_frame;
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

template<typename T> void ElementTypeSetup::set_attr(const Attribute<T>& attr, const T& value)
{
	globals->elem_type_attr_table.set_attr(type->type_attrs, attr, value);
}

inline Id get_last_in_subtree(const Frame& frame, Id root_elem_id)
{
	const Id sibling = get_next_sibling(frame, root_elem_id);
	if (sibling)
	{
		return (sibling - 1);
	}
	else
	{
		// TODO: maybe store the next subtree start offset in the sibling offset as well
		// use positive as its own sibling but negative as the non-sibling subtree?
		const auto root_elem_idx = id_to_index(root_elem_id);
		const auto depth = frame.elements[root_elem_idx].depth;
		size_t elem_idx = root_elem_idx + 1;
		for (; elem_idx < frame.elements.size(); elem_idx++)
		{
			if (frame.elements[elem_idx].depth <= depth) break;
		}
		return index_to_id(elem_idx - 1);
	}
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
