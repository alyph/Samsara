
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

class Shader
{
public:
	~Shader();
	static Shader create(const ShaderDesc& desc);

	Shader() = default;
	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;
	Shader(Shader&& other);
	Shader& operator=(Shader&& other);

	Id id() const { return shader_id; }

	int uniform_loc(const char* name) const;
	int attribute_loc(const char* name) const;

private:
	Id shader_id{};
};
