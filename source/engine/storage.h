#pragma once

#include "assertion.h"
#include <cstdint>
#include <vector>

// struct StorageRefCounter
// {
// 	using RefFunc = void(*)(uint32_t addr);
// 	RefFunc add;
// 	RefFunc release;
// };

template<typename T>
struct StorageData
{

	std::vector<int> ref_counts;
	std::vector<T> buffer;
	uint32_t available_slot_plus_one = 0;
};

template<typename T>
class Handle
{
public:
	Handle() = default;
	Handle(nullptr_t) {}

	Handle(const Handle<T>& other);
	Handle(Handle<T>&& other);

	Handle& operator=(const Handle<T>& other);
	Handle& operator=(Handle<T>&& other);

	T& get() const
	{
		asserts(data);
		auto& ref_counts = data->ref_counts;
		asserts(slot < ref_counts.size());
		asserts(ref_counts[slot] > 0);
		return data->buffer[slot];
	}

	T& operator*() const { return get(); }
	T* operator->() const { return &get();}

	operator bool() const { return (slot && data); }
	bool operator==(const Handle<T>& other) const { return (slot == other.slot && data == other.data); } // release_slot can be different?
	bool operator!=(const Handle<T>& other) const { return !(*this == other); }

	~Handle() { release(); }

	using ReleaseSlotFunc = void(*)(StorageData<T>*, uint32_t);

	uint32_t slot{};
	StorageData<T>* data{};
	ReleaseSlotFunc release_slot{};

private:
	void release();
};


template<typename T>
Handle<T>::Handle<T>(const Handle<T>& other)
{
	*this = other;
}

template<typename T>
Handle<T>::Handle<T>(Handle<T>&& other)
{
	*this = std::move(other);
}

template<typename T>
Handle<T>& Handle<T>::operator=(const Handle<T>& other)
{
	release();
	slot = other.slot;
	data = other.data;
	release_slot = other.release_slot;
	if (data)
	{
		auto& ref_counts = data->ref_counts;
		asserts(slot < ref_counts.size());
		asserts(ref_counts[slot] > 0);
		ref_counts[slot]++;
	}
	return *this;
}

template<typename T>
Handle<T>& Handle<T>::operator=(Handle<T>&& other)
{
	release();
	slot = other.slot;
	data = other.data;
	release_slot = other.release_slot;
	other.slot = 0;
	other.data = nullptr;
	other.release_slot = nullptr;
	return *this;
}

template<typename T>
void Handle<T>::release()
{
	if (data)
	{
		auto& ref_counts = data->ref_counts;
		asserts(slot < ref_counts.size());
		asserts(ref_counts[slot] > 0);
		ref_counts[slot]--;

		if (ref_counts[slot] == 0)
		{
			release_slot(data, slot);
		}
	}
	data = nullptr;
}


template<typename T>
class Storage
{
public:
	template<typename... Args>
	Handle<T> create(Args&&... args)
	{
		Handle handle;
		handle.slot = 0;
		handle.data = &data_block;

		// this guarantees to call the destructor in the same code unit where the constructor is called.
		handle.release_slot = &Storage<T>::release_slot;
				
		// find the appropriate slot
		if (data_block.available_slot_plus_one > 0)
		{
			auto slot = handle.slot = (data_block.available_slot_plus_one - 1);
			// construct new object
			// can do a placement new, but should have the same effects.
			T obj(std::forward<Args>(args)...);
			data_block.buffer[slot] = std::move(obj);
			// free slot's ref_count stores a linked list of available slots, 
			// pointing to the next -(available_slot + 1), 0 means no available
			data_block.available_slot_plus_one = -data_block.ref_counts[slot]; 
			data_block.ref_counts[slot] = 1;	
		}
		else // no available
		{
			auto slot = handle.slot = data_block.buffer.size();
			data_block.buffer.emplace_back(args...);
			data_block.ref_counts.push_back(1);
		}
		return handle;
	}

private:

	static void release_slot(StorageData<T>* data, uint32_t slot)
	{
		// Move it out as if deleting
		T dump = std::move(data->buffer[slot]);
		// now ref_count will be used to store the linked list
		// point to the current available slot + 1, which could be 0
		data->ref_counts[slot] = -data->available_slot_plus_one;
		data->available_slot_plus_one = (slot + 1);

		// at this point the object will be destructed.
		// the one in storage will be in a semi-initial state.
	}
	
	StorageData<T> data_block;


	// static std::vector<int> ref_counts;
	// static std::vector<T> buffer;
	// //static Storage<T> StaticStorage;
	// static inline StorageRefCounter counter{ &Storage<T>::add_ref, &Storage<T>::release_ref };
};

// template<typename T>
// std::vector<int> Storage<T>::ref_counts;

// template<typename T>
// std::vector<T> Storage<T>::buffer;

