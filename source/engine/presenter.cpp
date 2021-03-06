#include "presenter.h"
#include "easy/profiler.h"

struct PresentWorker
{
	struct ElementWorkerEntry
	{
		Id elem_id{};
	};

	struct ScopeWorkerEntry
	{
		size_t scope_idx{};
	};

	struct Section
	{
		Id root{};
		Id last_elem_id{};
	};

	struct MouseInteractElemEntry
	{
		Id curr_elem_guid{};
		Id prev_elem_guid{};
	};

	struct MouseInteractEntry
	{
		MouseInteractElemEntry interacts[(size_t)MouseInteraction::max]{};
	};

	std::vector<ElementWorkerEntry> elem_worker_stack;
	std::vector<ScopeWorkerEntry> scope_worker_stack;
	std::vector<Id> all_section_roots;
	std::vector<MouseInteractEntry> mouse_interact_per_depth;
	uint16_t num_mouse_interact_depth[(size_t)MouseInteraction::max * 2]{};
	Id last_finalized_elem{};

	inline bool is_mouse_currently(MouseInteraction interaction)
	{
		return num_mouse_interact_depth[(size_t)interaction * 2] > 0;
	}

	inline bool is_mouse_currently(MouseInteraction interaction, Id elem_guid)
	{
		const uint16_t depth = num_mouse_interact_depth[(size_t)interaction * 2];
		for (uint16_t i = 0; i < depth; i++)
		{
			if (mouse_interact_per_depth[i].interacts[(size_t)interaction].curr_elem_guid == elem_guid)
			{
				return true;
			}
		}
		return false;
	}

	inline bool is_mouse_currently(MouseInteraction interaction, const Element& elem)
	{
		return (elem.depth < num_mouse_interact_depth[(size_t)interaction * 2]) && 
			(mouse_interact_per_depth[elem.depth].interacts[(size_t)interaction].curr_elem_guid == elem.guid);
	}

	// TODO: should remove this and just use is_mouse_currently(interaction)
	inline bool is_mouse_currently_not(MouseInteraction interaction)
	{
		return num_mouse_interact_depth[(size_t)interaction * 2] == 0;
	}

	inline bool was_mouse_previously(MouseInteraction interaction, const Element& elem)
	{
		return (elem.depth < num_mouse_interact_depth[(size_t)interaction * 2 + 1]) && 
			(mouse_interact_per_depth[elem.depth].interacts[(size_t)interaction].prev_elem_guid == elem.guid);
	}
};

namespace attrs
{
	Attribute<FinalizerFunc> finalizer{nullptr};
	Attribute<RaycasterFunc> raycaster{nullptr};
	Attribute<RendererFunc> renderer{nullptr};
	Attribute<ElementPlacement> placement{ElementPlacement::structured};
	Attribute<Scalar> top{undefined_scalar};
	Attribute<Scalar> bottom{undefined_scalar};
	Attribute<Scalar> left{undefined_scalar};
	Attribute<Scalar> right{undefined_scalar};
	Attribute<Scalar> width{undefined_scalar};
	Attribute<Scalar> height{undefined_scalar};
	Attribute<Mat44> transform{Mat44::identity()};
	Attribute<Color> background_color{Color{0.f, 0.f, 0.f, 0.f}};
	Attribute<Color> foreground_color{Color{1.f, 1.f, 1.f, 1.f}};
	Attribute<Id> texture{null_id};
	Attribute<Id> shader{null_id};
	Attribute<String> text{String{}};
}

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

Id make_element(const Context& context, Id type_id)
{
	using offset_type = decltype(Element::tree_offset);

	auto frame = context.frame;
	auto worker = context.worker;
	auto globals = frame->globals;

	size_t new_elem_idx = frame->elements.size();
	auto& elem_worker = worker->elem_worker_stack.back();
	const auto depth = (worker->elem_worker_stack.size() - 1);
	if (elem_worker.elem_id)
	{
		// siblings of current element
		auto prev_sibling_idx = id_to_index(elem_worker.elem_id);
		auto& prev_sibling_elem = frame->elements[prev_sibling_idx];
		// TODO: replace this with a next tree offset
		prev_sibling_elem.tree_offset = static_cast<offset_type>(new_elem_idx - prev_sibling_idx);
	}
	// NOTE: commented out since this is kinda already checked in ScopedChildrenBlock::ScopedChildrenBlock
	// else
	// {
	// 	// first child element of the parent
		
	// 	// make sure the previous element is the parent
	// 	asserts(worker->elem_worker_stack.size() == 1 || 
	// 		(id_to_index(worker->elem_worker_stack[worker->elem_worker_stack.size() - 2].elem_id) 
	// 			== (new_elem_idx - 1)));
	// }

	auto& scope = globals->scopes[context.scope_idx];
	if (!scope.elem_guid)
	{
		// create a new global element
		scope.elem_guid = index_to_id(globals->global_elems.size());
		auto& global_elem = globals->global_elems.emplace_back();
		frame->presented.resize(globals->global_elems.size()); // 1 bit per each global element
		if (depth > 0)
		{
			const Id parent_id = worker->elem_worker_stack[depth - 1].elem_id;
			const Id parent_guid = frame->elements[id_to_index(parent_id)].guid;
			asserts(parent_guid <= 0xffffffff);
			global_elem.parent = static_cast<uint32_t>(parent_guid);
		}
	}

	// TODO: store the elem_id in the context to mark that an element has taken this place along with the scope and global guid
	// so next time if another make_element call to the context with a valid elem_id, we can assert

	auto& new_elem = frame->elements.emplace_back();
	new_elem.guid = scope.elem_guid;
	new_elem.type = static_cast<decltype(new_elem.type)>(type_id);
	new_elem.depth = static_cast<decltype(new_elem.depth)>(worker->elem_worker_stack.size() - 1);
	// TODO: if we store the depth in global element, we should assert the global and local elements' depth match

	// mark this element presented
	const auto presented_idx = id_to_index(new_elem.guid);
	asserts(!frame->presented.get(presented_idx)); // a global element can only be presented once per frame (if this is tripped, somewhere the context has messed up)
	frame->presented.set(presented_idx, true);

	const auto new_elem_id = index_to_id(new_elem_idx);
	if (type_id && globals->elem_types[id_to_index(type_id)].as_section_root)
	{
		worker->all_section_roots.push_back(new_elem_id);
	}
	return (elem_worker.elem_id = new_elem_id);
}

Id get_parent(const Frame& frame, Id elem_id)
{
	const auto elem_idx = id_to_index(elem_id); // NOTE: this can be 0
	const auto depth = frame.elements[elem_idx].depth;
	for (int64_t i = elem_idx - 1; i >= 0; i--)
	{
		if (frame.elements[i].depth < depth)
		{
			return index_to_id(i);
		}
	}
	return null_id;
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
	const auto& elem = frame.elements[elem_idx];
	const auto offset_idx = (elem_idx + elem.tree_offset);
	asserts(offset_idx > elem_idx);
	if (offset_idx < frame.elements.size())
	{
		const auto offset_depth = frame.elements[offset_idx].depth;
		if (elem.depth == offset_depth)
		{
			return index_to_id(offset_idx);
		}
		// can't offset to something lower 
		asserts(elem.depth > offset_depth);
	}
	return null_id;
}

Context create_scoped_context(const Context& parent_scope_context, uint64_t count)
{
	auto frame = parent_scope_context.frame;
	auto worker = parent_scope_context.worker;
	auto globals = frame->globals;
	
	Context context;
	context.frame = frame;
	context.worker = worker;
	context.begin_elem_id = (frame->elements.size() + 1);

	const auto parent_scope_idx = parent_scope_context.scope_idx;
	const auto& parent_scope = globals->scopes[parent_scope_idx];
	const auto new_depth = (parent_scope.depth + 1);
	const Id new_id = count;

	// scope stack size always matches the new context's scope depth 
	// note the root scope (depth 0) is not in the stack
	// stack must be at least the same size as the parent's depth, 
	// if we are rewinding then it could be larger, in that case
	// we could discard the rewound portion of the scope stack
	// regardless in which case, we will now make the stack match 
	// the new depth
	asserts(worker->scope_worker_stack.size() >= (new_depth - 1)); 
	worker->scope_worker_stack.resize(new_depth);
	auto& scope_worker = worker->scope_worker_stack.back();
	
	size_t last_visit = scope_worker.scope_idx ? scope_worker.scope_idx : (parent_scope_idx + 1);
	size_t parent_scope_end = (parent_scope_idx + parent_scope.next_offset);
	size_t insert = parent_scope_end;

	// NOTE: scope local ids are sorted (smaller to bigger) among siblings
	for (size_t idx = last_visit; idx < parent_scope_end;)
	{
		const auto& scope = globals->scopes[idx];
		if (new_id == scope.local_id)
		{
			context.scope_idx = idx;
			break;
		}
		else if (new_id < scope.local_id)
		{
			insert = idx;
			break;
		}
		idx += scope.next_offset;
	}

	// if the new scope local id is smaller than the last visit, 
	// then we still need search for the first half
	// TODO: since the loop body is essentially the same, maybe just merge with the above loop somehow
	if (insert == last_visit)
	{
		for (size_t idx = (parent_scope_idx + 1); idx < last_visit;)
		{
			const auto& scope = globals->scopes[idx];
			if (new_id == scope.local_id)
			{
				context.scope_idx = idx;
				break;
			}
			else if (new_id < scope.local_id)
			{
				insert = idx;
				break;
			}
			idx += scope.next_offset;
		}
	}

	if (!context.scope_idx)
	{
		// insert a new scope at the found insert position
		ScopeEntry new_scope;
		new_scope.local_id = new_id;
		new_scope.next_offset = 1;
		new_scope.depth = new_depth;
		globals->scopes.insert(globals->scopes.begin()+insert, new_scope);
		context.scope_idx = insert;

		// and increment offset of all ancestors
		// note can always loop over the entire global scope list to find the ancestors
		// by comparing the next offset with the insert position (if >=) and depth (if <)
		// but since we have the stack here, can just loop over that conveniently
		globals->scopes[0].next_offset++;
		for (size_t i = 0; i < new_depth-1; i++)
		{
			const size_t idx = worker->scope_worker_stack[i].scope_idx;
			globals->scopes[idx].next_offset++;
		}
	}

	scope_worker.scope_idx = context.scope_idx;
	return context;
}

Context create_scoped_context(const Context& parent_scope_context, uint64_t count, uint64_t user_id)
{
	// TODO: verify below is valid
	// just two create_scoped_context with a single count each?
	// TODO: if there are a lot of these user id items, maybe we could optimize it 
	// such that it doesn't require to have a distinctive scope per each user id
	Context intermediate_context = create_scoped_context(parent_scope_context, count);
	return create_scoped_context(intermediate_context, user_id);
}

Id get_working_elem_id(const Context& context)
{
	auto frame = context.frame;
	auto worker = context.worker;

	// right now we are just getting the current working element
	// TODO: we may want to check the current scope make sure we are not setting attr of 
	// the elements that are created in previously called functions
	auto& elem_worker = worker->elem_worker_stack.back();
	return elem_worker.elem_id;
}

Element& get_working_elem(const Context& context)
{
	auto frame = context.frame;
	auto worker = context.worker;

	// right now we are just getting the current working element
	// TODO: we may want to check the current scope make sure we are not setting attr of 
	// the elements that are created in previously called functions
	auto& elem_worker = worker->elem_worker_stack.back();
	asserts(elem_worker.elem_id);
	return frame->elements[id_to_index(elem_worker.elem_id)];
}

inline void close_tree_offset(Frame& frame, Id elem_id)
{
	using offset_type = decltype(Element::tree_offset);
	const auto elem_idx = id_to_index(elem_id);
	auto& elem = frame.elements[elem_idx];
	asserts(elem.tree_offset == 0 || elem.tree_offset == (frame.elements.size() - elem_idx));
	elem.tree_offset = static_cast<offset_type>(frame.elements.size() - elem_idx);
}

void finalize(const Context& context, bool open_tree)
{
	auto worker = context.worker;
	auto frame = context.frame;

	if (!open_tree && !worker->elem_worker_stack.empty())
	{
		const Id closing_id = worker->elem_worker_stack.back().elem_id;
		asserts(closing_id >= context.begin_elem_id);
		close_tree_offset(*frame, closing_id);
	}

	const Id prev = worker->last_finalized_elem;
	const Id first = worker->last_finalized_elem + 1;
	const Id last = frame->elements.size();
	// if (first > last)
	// {
	// 	return;
	// }

	const auto& section_roots = worker->all_section_roots;
	std::vector<Id> section_root_stack;
	size_t next_root_idx = 0;
	for (; next_root_idx < section_roots.size(); next_root_idx++)
	{
		const Id root_id = section_roots[next_root_idx];
		const auto& root_elem = get_element(*frame, root_id);

		if (root_id >= first)
		{
			break;
		}
		// 0 means not processed, thus contain everything afterwards
		// NOTE: we count everything that include prev finalized element,
		// so we can put those in stack and later pop to ensure a final
		// processing even if no elements are being finalized
		else if (root_elem.tree_offset == 0 || (root_id + root_elem.tree_offset > prev))
		{
			section_root_stack.push_back(root_id);
		}
	}

	Id current = first;
	while (current <= last || !section_root_stack.empty())
	{
		if (section_root_stack.empty())
		{
			if (next_root_idx < section_roots.size())
			{
				const Id next_root_id = section_roots[next_root_idx++];
				current = next_root_id + 1;
				section_root_stack.push_back(next_root_id);
			}
			else
			{
				break;
			}
		}
		else
		{
			const Id root_id = section_root_stack.back();
			const auto& root_elem = get_element(*frame, root_id);
			Id process_end{};
			bool always_finalize = false;
			// next root is the child of current root
			if (next_root_idx < section_roots.size() && 
				(root_elem.tree_offset == 0 || 
				(root_id + root_elem.tree_offset > section_roots[next_root_idx])))
			{
				const Id next_root_id = section_roots[next_root_idx++];
				// process everything from current to next_root_id
				// NOTE: next_root_id will be finalized by current section not by itself
				process_end = next_root_id;
				section_root_stack.push_back(next_root_id);
			}
			// current root has capped offset (i.e. all children have been presented)
			else if (root_elem.tree_offset)
			{
				// process until the end of the current root's sub tree
				process_end = (root_id + root_elem.tree_offset - 1);
				// we will call finalizer even if current > process_end becauase it is
				// required to call finalizer at least once last time for final processing 
				always_finalize = true;
				section_root_stack.pop_back();
			}
			// current root is uncapped (more children may be added in the future)
			else
			{
				process_end = last;
				// clear all stack because we will exit the loop after the finalize call
				section_root_stack.clear();
			}

			if (always_finalize || current <= process_end)
			{
				const auto finalizer_func = get_elem_attr_or_default(*frame, root_id, attrs::finalizer);
				if (finalizer_func)
				{
					finalizer_func(*frame, root_id, current, process_end);
				}
			}
			current = process_end + 1;
		}
	}
	worker->last_finalized_elem = last;
}

Id get_section_root(const Context& context)
{
	auto worker = context.worker;
	asserts(!worker->elem_worker_stack.empty());
	const Id working_elem_id = worker->elem_worker_stack.back().elem_id;
	const auto& section_roots = worker->all_section_roots;
	for (auto it = section_roots.rbegin(); it != section_roots.rend(); ++it)
	{
		const Id root_id = *it;
		const auto& root_elem = get_element(*context.frame, root_id);
		if (root_id < working_elem_id && 
			(root_elem.tree_offset == 0 || 
			root_id + root_elem.tree_offset > working_elem_id))
		{
			return root_id;
		}
	}
	return null_id;
}

bool is_elem_hover(const Context& context)
{
	const auto& elem = get_working_elem(context);
	auto worker = context.worker;
	return worker->is_mouse_currently(MouseInteraction::hover, elem);
}

bool is_elem_down(const Context& context, MouseInteraction interaction)
{
	const auto& elem = get_working_elem(context);
	auto worker = context.worker;
	return worker->is_mouse_currently(interaction, elem);
}

bool was_elem_clicked(const Context& context)
{
	const auto& elem = get_working_elem(context);
	auto worker = context.worker;

	// now up
	// previously down
	// now hover (releasing on a different element is a miss click)
	return 
		worker->is_mouse_currently_not(MouseInteraction::left_down) &&
		worker->was_mouse_previously(MouseInteraction::left_down, elem) &&
		worker->is_mouse_currently(MouseInteraction::hover, elem);
}

bool was_elem_pressed(const Context& context)
{
	const auto& elem = get_working_elem(context);
	auto worker = context.worker;

	// previously not down on this elem: up or down on other elem (can happen, if we missed a mouse up event)
	// now down on this elem
	return
		!worker->was_mouse_previously(MouseInteraction::left_down, elem) &&
		worker->is_mouse_currently(MouseInteraction::left_down, elem);
}

// states

static inline size_t get_state_ptr_idx(Buffer& buffer, ElementStateHeader* header)
{
	const auto header_ptr = reinterpret_cast<uint8_t*>(header);
	asserts(header_ptr >= buffer.data);
	return (header_ptr - buffer.data) + state_header_size;
}

static inline ElementStateHeader* first_state(Buffer& buffer)
{
	return buffer.empty() ? nullptr : reinterpret_cast<ElementStateHeader*>(buffer.data);
}

static inline ElementStateHeader* next_state(Buffer& buffer, ElementStateHeader* current_state)
{
	const auto state_data_ptr = get_state_ptr_idx(buffer, current_state);
	const auto next_state_ptr = state_data_ptr + current_state->size;
	return next_state_ptr >= buffer.size() ? nullptr : reinterpret_cast<ElementStateHeader*>(buffer.get(next_state_ptr));
}

static inline ElementStateHeader* get_state_header(Buffer& buffer, size_t state_data_ptr)
{
	asserts(state_data_ptr >= state_header_size);
	return reinterpret_cast<ElementStateHeader*>(buffer.get(state_data_ptr - state_header_size));
}

static inline ElementStateHeader* find_focused_state(Buffer& buffer, Id state_type_id, size_t state_size)
{
	for (auto header = first_state(buffer); header; header = next_state(buffer, header))
	{
		if (header->mode == ElementStateMode::focused && header->type_id == state_type_id)
		{
			asserts(header->size >= state_size);
			return header;
		}
	}
	return nullptr;
}

static inline ElementStateHeader* create_state_buffer(Buffer& buffer, Id state_type_id, size_t state_size, ElementStateMode mode)
{
	const auto old_size = buffer.size();
	asserts(Buffer::is_aligned(old_size));
	const auto data_size = Buffer::get_next_aligned(state_size);
	buffer.resize(old_size + state_header_size + data_size);
	asserts(Buffer::is_aligned(buffer.size()));
	std::memset(buffer.get(old_size + state_header_size), 0, data_size);
	auto header = reinterpret_cast<ElementStateHeader*>(buffer.get(old_size));
	asserts(data_size < std::numeric_limits<uint32_t>::max());
	asserts(state_type_id < std::numeric_limits<uint16_t>::max());
	header->size = static_cast<uint32_t>(data_size);
	header->type_id = static_cast<uint16_t>(state_type_id);
	header->mode = mode;
	return header;
}

void* access_elem_state_buffer(const Frame& frame, Id elem_id, Id state_type_id, size_t state_size, ElementStateMode mode, bool& out_new)
{
	out_new = false;
	auto globals = frame.globals;
	Buffer& buffer = globals->states_buffer;
	const Element& elem = get_element(frame, elem_id);
	GlobalElement& global_elem = frame.globals->global_elems[id_to_index(elem.guid)];
	if (mode == ElementStateMode::focused)
	{
		// TODO: apply some limitations on when you can access the focused state data
		// this is because all elements of same state data type share the same block of buffer for focused mode,
		// so we have to be careful not accidentally give the same pointer to multiple different elements
		// here you have to be either the current focused element and no one takes next focus
		// or you are the next focus but no one is currently focused
		asserts((frame.curr_focused_elem == elem.guid && (frame.next_focused_elem == null_id || frame.next_focused_elem == elem.guid)) ||
			(frame.next_focused_elem == elem.guid && frame.curr_focused_elem == null_id));
	}

	if (!global_elem.state_ptr)
	{
		ElementStateHeader* header{};
		if (mode == ElementStateMode::focused)
		{
			header = find_focused_state(buffer, state_type_id, state_size);
		}
		if (!header)
		{
			out_new = true;
			header = create_state_buffer(buffer, state_type_id, state_size, mode);
		}
		const auto state_ptr = get_state_ptr_idx(buffer, header);
		asserts(state_ptr && state_ptr < std::numeric_limits<decltype(global_elem.state_ptr)>::max());
		global_elem.state_ptr = static_cast<decltype(global_elem.state_ptr)>(state_ptr);
	}
	else
	{
		auto header = get_state_header(buffer, global_elem.state_ptr);
		asserts(header->type_id == state_type_id && header->size >= state_size && header->mode == mode);
	}
	return buffer.get(global_elem.state_ptr);
}

static Id next_state_type_id = 1;
Id new_state_type_id()
{
	return next_state_type_id++;
}

ScopedChildrenBlock::ScopedChildrenBlock(const Context& context):
	worker(context.worker),
	frame(context.frame)
{
	// make sure current working element is the last element registered (meaning only 1 children block is used per element)
	asserts(worker->elem_worker_stack.empty() ||
		(!context.frame->elements.empty() && 
		worker->elem_worker_stack.back().elem_id == index_to_id(context.frame->elements.size() - 1) &&
		context.frame->elements.back().tree_offset == 0)); // hierarchy must not be closed

	// add an empty entry, which will be filled when the first child element is made
	worker->elem_worker_stack.emplace_back();
}

ScopedChildrenBlock::~ScopedChildrenBlock()
{
	const auto elem_id = worker->elem_worker_stack.back().elem_id;
	if (elem_id)
	{
		close_tree_offset(*frame, elem_id);
	}
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

	// The root scope is never used by any actual elements
	// it is here so the root context can point to some scope
	// and conveniently scope index 0 can be treated as invalid index
	// since any actual scope must have index > 0
	// TODO: the root context with this root scope is passed to the present() function
	// normally present function will call other components functions which is fine, but 
	// if for some reason the present() function directly calls the make_element() then this will fail!
	ScopeEntry& root_scope = globals.scopes.emplace_back();
	root_scope.next_offset = 1; // this coincides with the total number of the scopes including this one
	root_scope.depth = 0; // just to be explicit

	return globals;
}

Presenter::Presenter()
{
	globals = make_globals();
	curr_frame.globals = &globals;
	curr_frame.presented.alloc(0, 1024, app_allocator()); // TODO: specify allocator for the presenter
	// curr_frame.engine = &engine;
}

void Presenter::set_present_func(PresentFunc func, void* param)
{
	present_func = func;
	present_func_param = param;
}

void Presenter::process_control(const std::vector<InputEvent>& events)
{
	// If we have never presented, present once so the input can test against the initial content
	// TODO: do we always need a present first? (in case state changed)
	if (curr_frame.frame_id == 0) { present(); }

	for (const auto& event : events)
	{
		// per each input event, do the following:

		// 1. process to find the new state of the UI, which includes:
		// - pressed mouse buttons and keys
		// - mouse pos
		// - pressed down item (lowest one)
		// - hovered item (lowest one)
		// - click (most importantly last clicks and timing, for checking double click)

		if (event.type == InputEventType::mouse_move)
		{
			latest_input.mouse_x = event.x;
			latest_input.mouse_y = event.y;
		}
		else if (event.type == InputEventType::mouse_wheel)
		{
			latest_input.mouse_wheel_pos += event.delta;
		}
		else if (event.type == InputEventType::character)
		{
			// each character input will trigger a present() call
			// TODO: if optimization needed, can merge with the key down input
			// although be aware of that there are repeated character input as well and we still need handle those separately
			latest_input.char_code = event.char_code;
		}
		else
		{
			int idx{}, bit{};
			if (event.type == InputEventType::key)
			{
				idx = (int)event.key / 64;
				bit = (int)event.key % 64;
			}
			else if (event.type == InputEventType::mouse_button)
			{
				static_assert((int)Keys::max <= (256-8));
				static_assert((int)MouseButtons::max <= 8);

				idx = 3;
				bit = 56 + (int)event.button;
			}
			else
			{
				asserts(false); // unknown events
				continue;
			}

			uint64_t mask = (((uint64_t)1ull) << bit);
			if (event.down) { latest_input.key_button_down[idx] |= mask; }
			else { latest_input.key_button_down[idx] &= (~mask); }
		}

		// always check mouse hover target
		RaycastResult hit_result = raycast(curr_frame, latest_input.mouse_x, latest_input.mouse_y);
		latest_input.mouse_interact_elems[(size_t)MouseInteraction::hover] = hit_result.hit_elem_id;
		latest_input.mouse_hit = hit_result;

		// TODO: static assert MouseInteraction::left_down == MouseButtons::left
		// and same for right and middle
		if (event.type == InputEventType::mouse_button && (int)event.button < 3)
		{
			latest_input.mouse_interact_elems[(int)event.button] = (event.down ? hit_result.hit_elem_id : null_id);
		}

		// TODO: clicks
		// clicks may be a derived state, maybe process that in the present() function 
		// when it has access to both previous and current input?

		// 2. call present()
		present();

		// TODO: verify if we actually need this
		// 3. [probably not needed] present again (call process_view?) since the state may have changed? (or maybe only do that for the last input)
	}
}

void Presenter::step_frame(const Time& time)
{
	EASY_FUNCTION();

	// advance to the latest time
	latest_time = time.current_time_seconds();

	// call do_present()
	present();
	
	// evaluate if some of the UI state may change as a result of present
	// state which may change: hover items
	// if such state changed, present() again (but to a limited number of times: just once for now)
	RaycastResult hit_result = raycast(curr_frame, latest_input.mouse_x, latest_input.mouse_y);
	// always store the new hit result, so even if we don't re-present() right away
	// we will get fresh values next time we present()
	latest_input.mouse_hit = hit_result;
	// TODO: do we need compare other parts of the hit result e.g. uv, iuv etc.
	if (hit_result.hit_elem_id != latest_input.mouse_interact_elems[(size_t)MouseInteraction::hover])
	{
		latest_input.mouse_interact_elems[(size_t)MouseInteraction::hover] = hit_result.hit_elem_id;
		present();
	}

	// call render() to render the latest frame out
	render(curr_frame);
}

void Presenter::present()
{
	EASY_FUNCTION();

	// keep the data of previous frame and previous UI state (the state when previous frame was presented)

	// prepare the data for the new frame
	curr_frame.frame_id++;
	curr_frame.elements.clear();
	curr_frame.inst_attr_table.clear();
	curr_frame.post_attr_table.clear();
	curr_frame.prev_time = curr_frame.curr_time;
	curr_frame.curr_time = latest_time;
	curr_frame.prev_input = curr_frame.curr_input;
	curr_frame.curr_input = latest_input;
	latest_input.char_code = 0; // char_code only exist for one frame

	curr_frame.prev_focused_elem = curr_frame.curr_focused_elem;
	curr_frame.curr_focused_elem = curr_frame.next_focused_elem; // next_focused_elem then remains the same until changed
	// remove current focus if it wasn't rendered in the last frame
	if (curr_frame.next_focused_elem && !curr_frame.presented.get(id_to_index(curr_frame.next_focused_elem)))
	{
		curr_frame.next_focused_elem = null_id;
	}

	// TODO: to avoid extra allocation, maybe just keep a worker around and clear everything every frame
	PresentWorker worker;
	Context context;
	context.frame = &curr_frame;
	context.worker = &worker;

	// setup the mouse interaction cache
	for (int interact = 0; interact < (int)MouseInteraction::max; interact++)
	{
		for (int prev = 0; prev < 2; prev++)
		{
			const Id leaf_elem = (prev ? 
				curr_frame.prev_input.mouse_interact_elems[interact] : 
				curr_frame.curr_input.mouse_interact_elems[interact]);

			uint16_t num_depth = 0;
			Id elem_guid = leaf_elem;
			while (elem_guid)
			{
				num_depth++;
				elem_guid = globals.global_elems[id_to_index(elem_guid)].parent;
			}

			worker.num_mouse_interact_depth[interact*2+prev] = num_depth;

			if (worker.mouse_interact_per_depth.size() < num_depth)
				worker.mouse_interact_per_depth.resize(num_depth);

			elem_guid = leaf_elem;
			for (int depth = num_depth - 1; depth >= 0; depth--)
			{
				Id& interact_elem = (prev ? 
					worker.mouse_interact_per_depth[depth].interacts[interact].prev_elem_guid:
					worker.mouse_interact_per_depth[depth].interacts[interact].curr_elem_guid);
				interact_elem = elem_guid;
				elem_guid = globals.global_elems[id_to_index(elem_guid)].parent;
			}
		}
	}

	// clear next focus if mouse down on other elements
	// TODO: maybe should count right_down too
	if (curr_frame.next_focused_elem &&
		worker.is_mouse_currently(MouseInteraction::left_down) &&
		!worker.is_mouse_currently(MouseInteraction::left_down, curr_frame.next_focused_elem))
	{
		curr_frame.next_focused_elem = null_id;
	}

	// reset the presetned flags, and will be toggled on if presented in this frame
	curr_frame.presented.zero();

	// call present func to present
	{
		// Create a scoped children block to ensure all elements are children of this virtual root
		// This will insert an empty entry into the worker stack for the top level elements,
		// and when exiting the scope, this will also pop the last top level element, and set its
		// tree offset. (NOTE: the worker stack can never be empty during the present_func call.
		// The scope stack though will be empty at the beginning)
		const ScopedChildrenBlock root(context);
		asserts(present_func);
		present_func(std::move(context), present_func_param);
	}
	
	finalize(context);
	
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
			auto renderer = get_elem_attr_or_assert(frame, elem_id, attrs::renderer);
			asserts(renderer); // still need assert since the assigned value may be null

			// this renders entire sub tree
			renderer(frame, elem_id);

			// now moves to next sibling
			elem_id = get_next_sibling(frame, elem_id);
		}
	}
}

RaycastResult Presenter::raycast(const Frame& frame, double x, double y)
{
	RaycastResult closest_hit_result{};
	closest_hit_result.point.z = 2.0; // z is between [-1, 1], so 2 will be farther than the farthest

	Id elem_id = frame.elements.empty() ? null_id : index_to_id(0);
	while (elem_id)
	{
		auto raycaster = get_elem_attr_or_assert(frame, elem_id, attrs::raycaster);
		asserts(raycaster); // still need assert since the assigned value may be null

		RaycastResult result = raycaster(frame, elem_id, x, y);
		if (result.hit_elem_id && result.point.z < closest_hit_result.point.z)
		{
			closest_hit_result = result;
		}

		// now moves to next sibling
		elem_id = get_next_sibling(frame, elem_id);
	}

	if (closest_hit_result.hit_elem_id)
	{
		closest_hit_result.hit_elem_id = frame.elements[id_to_index(closest_hit_result.hit_elem_id)].guid;
	}

	return closest_hit_result;
}




