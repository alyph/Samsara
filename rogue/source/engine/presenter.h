
#pragma once

#include <vector>
#include "id.h"
#include "attribute.h"
#include "math_types.h"
#include "color.h"
#include "buffer.h"
#include <functional>

struct InputEvent;
struct Frame;
struct Context;
struct ScopedChildrenBlock;
struct PresentWorker;
struct ElementType;
struct ElementTypeSetup;

using RendererFunc = void(*)(const Frame& frame, Id elem_id);
using ElemTypeInitFunc = std::function<void(ElementTypeSetup&)>;

namespace attrs
{
	extern Attribute<RendererFunc> renderer;
	extern Attribute<double> top;
	extern Attribute<double> bottom;
	extern Attribute<double> left;
	extern Attribute<double> right;
	extern Attribute<double> width;
	extern Attribute<double> height;
	extern Attribute<Mat44> transform;
	extern Attribute<Color> background_color;
}

extern Id register_elem_type(ElemTypeInitFunc init_func);
//template<typename T> void set_elem_type_attr(Id type_id, const Attribute<T>& attr, const T& value);

extern Id make_element(const Context& context, Id type_id);
extern Id get_first_child(const Frame& frame, Id elem_id);
extern Id get_next_sibling(const Frame& frame, Id elem_id);
extern BufferReader get_elem_attr_buffer(const Frame& frame, Id elem_id, Id attr_id);
extern BufferWriter init_elem_attr_buffer(const Context& context, Id attr_id);
template<typename T> const T& get_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T* get_defined_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> const T& get_defined_elem_attr_asserted(const Frame& frame, Id elem_id, const Attribute<T>& attr);
template<typename T> void set_elem_attr(const Context& context, const Attribute<T>& attr, const T& val);

extern Context create_scoped_context(const Context& parent_scope_context, uint64_t count);
extern Context create_scoped_context(const Context& parent_scope_context, uint64_t count, uint64_t user_id);
//extern Id get_working_elem(const Context& context);

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define _ctx create_scoped_context(ctx, __COUNTER__)
#define _ctx_id(user_id) create_scoped_context(ctx, __COUNTER__, user_id)
#define _attr(attr, val) set_elem_attr(ctx, attr, val)
#define _children if (const ScopedChildrenBlock CONCAT(children_block_, __COUNTER__){ctx}; true)


struct AttrTableEntry
{
	uint16_t attr_id{};
	uint32_t buffer_ptr{};
};

struct PresentGlobals
{
	std::vector<ElementType> elem_types;
	std::vector<AttrTableEntry> elem_type_attr_table;
	std::vector<uint8_t> elem_type_attr_buffer;
};

struct Element
{
	uint16_t type{};
	uint16_t depth{};
	uint32_t sibling_offset{};
	uint32_t attr_table_index{};
	uint16_t num_attrs{};
};

struct ElementType
{
	Id id{};
	std::string name;
	uint32_t attr_table_index{};
	uint16_t num_attrs{};
};

struct ElementTypeSetup
{
	ElementType* type{};
	PresentGlobals* globals{};

	void set_name(const char* name);
	template<typename T> void set_attr(const Attribute<T>& attr, const T& value);
	BufferWriter init_attr_buffer(Id attr_id);

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
	std::vector<AttrTableEntry> attr_table;
	std::vector<uint8_t> attr_buffer;
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
	auto buffer = init_attr_buffer(attr.id);
	attribute_serialization::store(buffer, value);
}

template<typename T> 
const T& get_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	// TODO: call get_defined_elem_attr() first instead
	const auto buffer = get_elem_attr_buffer(frame, elem_id, attr.id);
	if (buffer)
	{
		const T* val{};
		attribute_serialization::load(buffer, val);
		return *val;
	}
	return attr.default_value;
}

template<typename T> 
const T* get_defined_elem_attr(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	const auto buffer = get_elem_attr_buffer(frame, elem_id, attr.id);
	if (buffer)
	{
		const T* val{};
		attribute_serialization::load(buffer, val);
		return val;
	}
	return nullptr;
}

template<typename T> 
const T& get_defined_elem_attr_asserted(const Frame& frame, Id elem_id, const Attribute<T>& attr)
{
	auto val = get_defined_elem_attr(frame, elem_id, attr);
	asserts(val);
	return *val;
}

template<typename T> 
void set_elem_attr(const Context& context, const Attribute<T>& attr, const T& val)
{
	auto buffer = init_elem_attr_buffer(context, attr.id);
	attribute_serialization::store(buffer, val);
}