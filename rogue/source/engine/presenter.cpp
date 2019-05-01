#include "presenter.h"
#include "easy/profiler.h"

struct PresentWorker
{
	struct ElementWorkerEntry
	{
		Id elem_id{};
	};

	std::vector<ElementWorkerEntry> elem_worker_stack;
};

namespace attrs
{
	Attribute<RendererFunc> renderer{nullptr};
	Attribute<double> top{0.0};
	Attribute<double> bottom{0.0};
	Attribute<double> left{0.0};
	Attribute<double> right{0.0};
	Attribute<double> width{0.0};
	Attribute<double> height{0.0};
	Attribute<Mat44> transform{Mat44::identity()};
	Attribute<Color> background_color{Color{}};
	Attribute<Id> texture{null_id};
	Attribute<Id> shader{null_id};
	Attribute<String> text{String{}};
}

// struct GlobalPresenterData
// {
// 	std::vector<ElementType> elem_types;
// 	std::vector<AttrTableEntry> elem_type_attr_table;
// 	std::vector<uint8_t> elem_type_attr_buffer;
// };
// static GlobalPresenterData global_data;

static std::vector<ElemTypeInitFunc>& registered_elem_type_funcs()
{
	static std::vector<ElemTypeInitFunc> funcs;
	return funcs;
}

Id register_elem_type(ElemTypeInitFunc init_func)
{
	auto& registered_funcs = registered_elem_type_funcs();
	const auto id = index_to_id(registered_funcs.size());
	registered_funcs.push_back(init_func);
	return id;
}

static Buffer& init_attr_table_and_buffer(Id attr_id, uint32_t& table_index, uint16_t& num_attrs, std::vector<AttrTableEntry>& table, Buffer& buffer)
{
	using attr_id_type = decltype(AttrTableEntry::attr_id);
	using buffer_ptr_type = decltype(AttrTableEntry::buffer_ptr);
	using table_index_type = std::remove_reference_t<decltype(table_index)>;

	const auto ptr = buffer.size();
	asserts(buffer.is_aligned(ptr));

	// find existing entry for the given attr and point it to the new buffer location
	// TODO: right now we just keep adding new buffer for each set attr value
	// but the buffer may contain multiple values for the same attr, even though
	// it still functions, the values other than the last one are not used and wasting space
	for (uint16_t i = 0; i < num_attrs; i++)
	{
		auto& table_entry = table[table_index + i];
		if (table_entry.attr_id == attr_id)
		{
			table_entry.buffer_ptr = static_cast<buffer_ptr_type>(ptr);
			return buffer;
		}
	}

	if (num_attrs == 0)
	{
		table_index = static_cast<table_index_type>(table.size());
		num_attrs = 1;
	}
	else
	{
		// make sure the table for each element is continguous
		asserts((table_index + num_attrs) == table.size());
		num_attrs++;
	}

	auto& table_entry = table.emplace_back();
	table_entry.attr_id = static_cast<attr_id_type>(attr_id);
	table_entry.buffer_ptr = static_cast<buffer_ptr_type>(ptr);
	return buffer;
}

void ElementTypeSetup::set_name(const char* name)
{
	type->name = name;
}

Buffer& ElementTypeSetup::init_attr_buffer(Id attr_id)
{
	return init_attr_table_and_buffer(attr_id, type->attr_table_index, type->num_attrs,
		globals->elem_type_attr_table, globals->elem_type_attr_buffer);
}

Id make_element(const Context& context, Id type_id)
{
	using sibling_offset_type = decltype(Element::sibling_offset);

	auto frame = context.frame;
	auto worker = context.worker;

	size_t new_elem_idx = frame->elements.size();
	auto& elem_worker = worker->elem_worker_stack.back();
	if (elem_worker.elem_id)
	{
		// siblings of current element
		auto prev_sibling_idx = id_to_index(elem_worker.elem_id);
		auto& prev_sibling_elem = frame->elements[prev_sibling_idx];
		prev_sibling_elem.sibling_offset = static_cast<sibling_offset_type>(new_elem_idx - prev_sibling_idx);
	}
	// else
	// {
	// 	// first child element of the parent
		
	// 	// make sure the previous element is the parent
	// 	asserts(worker->elem_worker_stack.size() == 1 || 
	// 		(id_to_index(worker->elem_worker_stack[worker->elem_worker_stack.size() - 2].elem_id) 
	// 			== (new_elem_idx - 1)));
	// }

	auto& new_elem = frame->elements.emplace_back();
	new_elem.type = static_cast<decltype(new_elem.type)>(type_id);
	new_elem.depth = static_cast<decltype(new_elem.depth)>(worker->elem_worker_stack.size() - 1);

	return (elem_worker.elem_id = index_to_id(new_elem_idx));
}

Id get_first_child(const Frame& frame, Id elem_id)
{
	const auto elem_idx = id_to_index(elem_id);
	const auto first_child_idx = (elem_idx + 1);
	if (frame.elements.size() > first_child_idx &&
		frame.elements[first_child_idx].depth > frame.elements[elem_idx].depth)
	{
		asserts(frame.elements[first_child_idx].depth == (frame.elements[elem_idx].depth + 1));
		return index_to_id(first_child_idx);
	}
	return null_id;
}

Id get_next_sibling(const Frame& frame, Id elem_id)
{
	const auto elem_idx = id_to_index(elem_id);
	const auto sibling_offset = frame.elements[elem_idx].sibling_offset;
	if (sibling_offset)
	{
		return index_to_id(elem_idx + sibling_offset);
	}
	return null_id;
}

static BufferBlock get_attr_buffer_from_table(Id attr_id, uint32_t table_index, uint16_t num_attrs, const std::vector<AttrTableEntry>& table, const Buffer& buffer)
{
	BufferBlock block;
	for (uint16_t i = 0; i < num_attrs; i++)
	{
		auto& table_entry = table[table_index + i];
		if (table_entry.attr_id == attr_id)
		{
			asserts(buffer.is_aligned(table_entry.buffer_ptr));
			const uint32_t next_idx = (table_index + i + 1);
			block.ptr = buffer.get(table_entry.buffer_ptr);
			block.size = (next_idx < table.size() ? (table[next_idx].buffer_ptr - table_entry.buffer_ptr) : (buffer.size() - table_entry.buffer_ptr));
			break;
		}
	}	
	return block;
}

BufferBlock get_elem_attr_buffer(const Frame& frame, Id elem_id, Id attr_id)
{
	const auto& elem = frame.elements[id_to_index(elem_id)];
	auto buffer = get_attr_buffer_from_table(attr_id, elem.attr_table_index, elem.num_attrs, frame.attr_table, frame.attr_buffer);
	if (!buffer && elem.type)
	{
		const auto& globals = *frame.globals;
		const auto& elem_type = globals.elem_types[id_to_index(elem.type)];
		buffer = get_attr_buffer_from_table(attr_id, elem_type.attr_table_index, elem_type.num_attrs, globals.elem_type_attr_table, globals.elem_type_attr_buffer);
	}
	return buffer;
}

Buffer& init_elem_attr_buffer(const Context& context, Id attr_id)
{
	auto frame = context.frame;
	auto worker = context.worker;

	// right now we are just getting the current working element
	// TODO: we may want to check the current scope make sure we are not setting attr of 
	// the elements that are created in previously called functions
	auto& elem_worker = worker->elem_worker_stack.back();
	asserts(elem_worker.elem_id);
	auto& elem = frame->elements[id_to_index(elem_worker.elem_id)];

	// NOTE: this will assert if the attr table is no longer continguous for the given element
	// meaning you have called _attr after _children block
	return init_attr_table_and_buffer(attr_id, elem.attr_table_index, elem.num_attrs, frame->attr_table, frame->attr_buffer);
}

Context create_scoped_context(const Context& parent_scope_context, uint64_t count)
{
	Context context;
	context.frame = parent_scope_context.frame;
	context.worker = parent_scope_context.worker;
	// TODO: scope id

	return context;
}

Context create_scoped_context(const Context& parent_scope_context, uint64_t count, uint64_t user_id)
{
	Context context;
	context.frame = parent_scope_context.frame;
	context.worker = parent_scope_context.worker;
	// TODO: scope id

	return context;
}

// static Id get_working_elem(const Context& context)
// {

// }

ScopedChildrenBlock::ScopedChildrenBlock(const Context& context):
	worker(context.worker)
{
	// make sure current working element is the last element registered (meaning only 1 children block is used per element)
	asserts(!context.frame->elements.empty() && 
		worker->elem_worker_stack.back().elem_id == index_to_id(context.frame->elements.size() - 1));

	// add an empty entry, which will be filled when the first child element is made
	worker->elem_worker_stack.emplace_back();
}

ScopedChildrenBlock::~ScopedChildrenBlock()
{
	worker->elem_worker_stack.pop_back();
}

static PresentGlobals make_globals()
{
	PresentGlobals globals;

	// setup element types based on globally registered init funcs		
	const auto& elem_type_funcs = registered_elem_type_funcs();
	globals.elem_types.resize(elem_type_funcs.size());
	ElementTypeSetup setup{nullptr, &globals};
	for (size_t i = 0; i < elem_type_funcs.size(); i++)
	{
		auto& elem_type = globals.elem_types[i];
		elem_type.id = index_to_id(i);
		setup.type = &elem_type;
		elem_type_funcs[i](setup);
	}

	return globals;
}

Presenter::Presenter()
{
	globals = make_globals();
	curr_frame.globals = &globals;
	// curr_frame.engine = &engine;
}

void Presenter::set_present_func(PresentFunc func, void* param)
{
	present_func = func;
	present_func_param = param;
}

void Presenter::process_control(const std::vector<InputEvent>& events)
{
	// maybe do an initial present first (in case state changed)

	// per each input event, do the following
	// 1. process to find the new state of the UI, which includes:
	// - pressed mouse buttons and keys
	// - mouse pos
	// - pressed down item (lowest one)
	// - hovered item (lowest one)
	// - click (most importantly last clicks and timing, for checking double click)

	// 2. call do_present()

	// 3. [probably not needed] present again (call process_view?) since the state may have changed? (or maybe only do that for the last input)
}

void Presenter::step_frame(double dt)
{
	EASY_FUNCTION();

	// increment time with dt
	time += dt;

	// call do_present()
	present();
	
	// evaluate if some of the UI state may change with the result of present
	// state which may change: down, hover items
	// if such state changed, do_present() again (but to a limited number of times: maybe 1)

	// call render() to render the latest frame out
	render(curr_frame);
}

void Presenter::present()
{
	EASY_FUNCTION();

	// keep the data of previous frame and previous UI state (the state when previous frame was presented)

	// prepare the data for the new frame
	// use the current UI state

	curr_frame.frame_id++;
	curr_frame.elements.clear();
	curr_frame.attr_table.clear();
	curr_frame.attr_buffer.clear();

	// call present func to present
	PresentWorker worker;
	// make an empty entry in elem stack for the top level elements (the stack can never be empty)
	worker.elem_worker_stack.emplace_back();

	Context context;
	context.frame = &curr_frame;
	context.worker = &worker;

	asserts(present_func);
	present_func(std::move(context), present_func_param);

	// may perform any necessary post processing
	// now new frame is stored in curr_frame
}

void Presenter::render(const Frame& frame)
{
	EASY_FUNCTION();

	// render the last frame presented
	if (frame.elements.size() > 0)
	{
		Id elem_id = index_to_id(0);
		while (elem_id)
		{
			const auto elem_idx = id_to_index(elem_id);
			const auto& elem = frame.elements[elem_idx];
			auto renderer = get_elem_attr(frame, elem_id, attrs::renderer);
			asserts(renderer);

			// this renders entire sub tree
			renderer(frame, elem_id);

			// now moves to next sibling
			elem_id = get_next_sibling(frame, elem_id);
		}
	}
}




