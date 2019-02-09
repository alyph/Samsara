#pragma once

#include "math_types.h"
#include "id.h"
#include "color.h"
#include "presenter.h"
#include <vector>
#include <memory>
#include <optional>

class Shader;

namespace attrs
{
	extern Attribute<Id> mesh_id;
}

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
	Id shader_id;
	int param_mvp;
};

class MeshStore
{
public:
	Id add_mesh(Mesh&& mesh, Id shader);
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

extern Id add_mesh(Mesh&& mesh, Id shader);

namespace elem
{
	extern Id mesh(const Context ctx);
}

