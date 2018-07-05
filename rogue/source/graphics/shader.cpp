
#include "shader.h"

#include <GL/glew.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

std::shared_ptr<Shader> Shader::create(const ShaderDesc& desc)
{
	// Read the Vertex Shader code from the file
	std::string vs_code;
	std::ifstream vs_stream(desc.vs_path, std::ios::in);
	if (vs_stream.is_open())
	{
		std::stringstream sstr;
		sstr << vs_stream.rdbuf();
		vs_code = sstr.str();
		vs_stream.close();
	}
	else
	{
		printf("Unable to open vertex shader file %s. Are you in the right directory?\n", desc.vs_path.c_str());
		return nullptr;
	}

	// Read the Fragment Shader code from the file
	std::string fs_code;
	std::ifstream fs_stream(desc.fs_path, std::ios::in);
	if (fs_stream.is_open())
	{
		std::stringstream sstr;
		sstr << fs_stream.rdbuf();
		fs_code = sstr.str();
		fs_stream.close();
	}
	else
	{
		printf("Unable to open fragment shader file %s. Are you in the right directory?\n", desc.fs_path.c_str());
		return nullptr;
	}
	
	auto compile_shader = [](GLuint id, const std::string& source, const std::string& path)
	{
		auto source_cstr = source.c_str();
		glShaderSource(id, 1, &source_cstr, nullptr);
		glCompileShader(id);
		GLint success{};
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (success)
		{
			return true;
		}
		else
		{
			GLint len{};
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
			std::vector<char> error(len+1);
			glGetShaderInfoLog(id, len, nullptr, error.data());
			error.back() = '\0';
			printf("Failed to compile shader %s, error: %s", path.c_str(), error.data());
			return false;
		}
	};

	// Create the shaders
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	const bool success = 
		compile_shader(vs, vs_code, desc.vs_path) &&
		compile_shader(fs, fs_code, desc.fs_path);

	if (!success)
	{
		glDeleteShader(vs);
		glDeleteShader(fs);
		return nullptr;
	}

	// Link the program
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	// Check the program
	std::shared_ptr<Shader> shader;
	GLint link_success;
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);
	if (link_success)
	{
		shader = std::make_shared<Shader>();
		shader->shader_id = program;
	}
	else
	{
		GLint len{};
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> error(len+1);
		glGetProgramInfoLog(program, len, nullptr, error.data());
		error.back() = '\0';
		printf("Failed to link program, error: %s", error.data());
	}
	
	glDetachShader(program, vs);
	glDetachShader(program, fs);
	
	glDeleteShader(vs);
	glDeleteShader(fs);

	return shader;
}

Shader::~Shader()
{
	glDeleteProgram(shader_id);
}

int Shader::uniform_loc(const char* name)
{
	return glGetUniformLocation(static_cast<GLuint>(shader_id), name);
}

int Shader::attribute_loc(const char* name)
{
	return glGetAttribLocation(static_cast<GLuint>(shader_id), name);
}

