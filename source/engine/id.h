#pragma once
#include <cstdint>
#include "assertion.h"

using Id = uint64_t;
static constexpr const Id null_id = 0;

inline size_t id_to_index(Id id)
{
	asserts(id != null_id);
	return (id - 1);
}

inline Id index_to_id(size_t index)
{
	return (index + 1);
}
