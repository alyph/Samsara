#pragma once

#include "string.h"
#include <filesystem>

namespace filesystem
{
	inline std::filesystem::path make_std_path(const String& path)
	{
		return std::filesystem::path{path.data(), path.data() + path.size()};
	}

	inline bool create_directories(const String& path)
	{
		return std::filesystem::create_directories(make_std_path(path));
	}

	inline bool exists(const String& path)
	{
		return std::filesystem::exists(make_std_path(path));
	}
}


