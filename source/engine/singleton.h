#pragma once

#include <vector>
#include <memory>

class ISingletonFactory;

extern size_t register_singleton(size_t size, size_t alignment, std::unique_ptr<ISingletonFactory>&& factory);

template<typename T>
class SingletonHandle
{
public:
	inline SingletonHandle();
	size_t ptr{};
};

class SingletonCollection
{
public:
	SingletonCollection();
	~SingletonCollection();

	template<typename T>
	inline T& get(const SingletonHandle<T>& handle);

private:
	uint8_t* buffer{};
};

class ISingletonFactory
{
public:
	virtual void construct(void* ptr) const = 0;
	virtual void destruct(void* ptr) const = 0;
};

template<typename T>
class SingletonFactory : public ISingletonFactory
{
private:
	virtual void construct(void* ptr) const override
	{
		new(ptr) T;
	}

	virtual void destruct(void* ptr) const override
	{
		reinterpret_cast<T*>(ptr)->~T();
	}
};

template<typename T>
inline SingletonHandle<T>::SingletonHandle()
{
	ptr = register_singleton(sizeof(T), alignof(T), std::make_unique<SingletonFactory<T>>());
}

template<typename T>
inline T& SingletonCollection::get(const SingletonHandle<T>& handle)
{
	return *reinterpret_cast<T*>(buffer + handle.ptr);
}
