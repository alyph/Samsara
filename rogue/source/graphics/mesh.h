#pragma once

#include "math/math_types.h"
#include "types/id.h"
#include "shader.h"
#include "color.h"
#include <vector>
#include <memory>
#include <optional>

class Mesh
{
public:
	std::vector<uint32_t> indices;
	std::vector<Vec3> vertices;
	std::vector<Color> colors;
};

struct MeshCache
{
	Id vao;
	uint32_t vert_count;
};

struct MeshShaderCache
{
	std::shared_ptr<Shader> shader;
	int param_mvp;
};

class MeshStore
{
public:
	std::optional<Id> add_mesh(Mesh&& mesh, const std::shared_ptr<Shader>& shader);
	const MeshCache& mesh_cache(Id mesh_id) const;
	const MeshShaderCache& shader_cache(Id mesh_id) const;
	

private:
	struct InternalMeshObject
	{
		~InternalMeshObject();

		Mesh mesh;
		MeshCache cache;
		size_t shader_cache_idx{};
		Id vertex_buffer{};
		Id color_buffer{};
	};

	std::vector<InternalMeshObject> mesh_objects;
	std::vector<MeshShaderCache> shader_caches;
};

struct MeshDrawItem
{
	Id mesh_id;
	Mat44 mvp;
};

class MeshDrawStream
{
public:
	std::vector<MeshDrawItem> items;
};

namespace renderer
{
	void draw(const MeshStore& store, const MeshDrawStream& stream);
}

