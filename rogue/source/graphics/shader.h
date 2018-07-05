
#pragma once

#include "types/id.h"
#include <string>
#include <vector>
#include <memory>

struct ShaderDesc
{
	std::string vs_path;
	std::string fs_path;

	std::vector<std::string> attributes;
	std::vector<std::string> uniforms;
};

class Shader
{
public:
	~Shader();
	static std::shared_ptr<Shader> create(const ShaderDesc& desc);

	Id id() { return shader_id; }

	int uniform_loc(const char* name);
	int attribute_loc(const char* name);

private:
	Id shader_id{};
};
