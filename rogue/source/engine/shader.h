
#pragma once

#include "id.h"
#include <string>
#include <vector>

struct ShaderDesc
{
	std::string vs_path;
	std::string fs_path;

	std::vector<std::string> attributes;
	std::vector<std::string> uniforms;
};

extern Id create_shader(const ShaderDesc& desc);
extern int shader_uniform_loc(Id id, const char* name);
extern int shader_attribute_loc(Id id, const char* name);
