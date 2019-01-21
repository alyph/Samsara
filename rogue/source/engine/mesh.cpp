#include "mesh.h"
#include "shader.h"
#include "assertion.h"
#include "viewport.h"
#include <GL/glew.h>

namespace attrs
{
	Attribute<Id> mesh_id{null_id};
}

static constexpr const char* uniform_mvp = "MVP";
static constexpr const char* attribute_pos = "VertexPos";
static constexpr const char* attribute_color = "VertexColor";

static MeshStore global_mesh_store;

Id MeshStore::add_mesh(Mesh&& mesh, Id shader)
{
	asserts(shader, "mesh shader must be valid.");
	asserts(!mesh.indices.empty(), "indices must not be empty.");
	asserts(!mesh.vertices.empty(), "vertices must not be empty.");

	// find or create a shader cache
	const auto shader_id = shader;
	size_t shader_cache_idx = -1;
	for (size_t i = 0; i < shader_caches.size(); i++)
	{
		if (shader_caches[i].shader_id == shader_id)
		{
			shader_cache_idx = i;
			break;
		}
	}

	if (shader_cache_idx == -1)
	{
		shader_cache_idx = shader_caches.size();
		auto& cache = shader_caches.emplace_back();
		cache.shader_id = shader_id;
		cache.param_mvp = shader_uniform_loc(shader, uniform_mvp);
	}

	const Id id = mesh_objects.size();
	auto& mesh_obj = mesh_objects.emplace_back();

	// create vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	mesh_obj.cache.vao = vao;
	mesh_obj.cache.vert_count = static_cast<uint32_t>(mesh.indices.size());

	// create and bind verts
	GLuint vbo;
	const auto vert_loc = shader_attribute_loc(shader, attribute_pos);
	if (vert_loc >= 0)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mesh.vertices[0]), mesh.vertices.data(), GL_STATIC_DRAW); // TODO: support some dynamic meshes
		glVertexAttribPointer(vert_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vert_loc); 
		mesh_obj.vertex_buffer = vbo;
	}
	else
	{
		printf("Cannot find shader attribute: %s\n", attribute_pos);
	}

	// create and bind colors
	const auto color_loc = shader_attribute_loc(shader, attribute_color);
	if (color_loc >= 0)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(mesh.colors[0]), mesh.colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(color_loc);
		mesh_obj.color_buffer = vbo;
	}
	else
	{
		printf("Cannot find shader attribute: %s\n", attribute_color);
	}

	// create and bind indices
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(mesh.indices[0]), mesh.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	mesh_obj.mesh = std::move(mesh);
	mesh_obj.shader_cache_idx = shader_cache_idx;

	return id;
}

const MeshCache& MeshStore::mesh_cache(Id mesh_id) const
{
	return mesh_objects[mesh_id].cache;
}

const MeshShaderCache& MeshStore::shader_cache(Id mesh_id) const
{
	const auto shader_idx = mesh_objects[mesh_id].shader_cache_idx;
	return shader_caches[shader_idx];
}

MeshStore::InternalMeshObject::~InternalMeshObject()
{
	// TODO: delete vao, vbo, ebo etc.
	// maybe should have a the members auto delete those.
}

namespace renderer
{
	void draw(const MeshStore& store, const MeshDrawStream& stream)
	{
		// TODO: revert states
		// opengl states
		// depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		for (const auto& item : stream.items)
		{
			const auto& mesh_cache = store.mesh_cache(item.mesh_id);
			const auto& shader_cache = store.shader_cache(item.mesh_id);

			// use shader
			glUseProgram(static_cast<GLuint>(shader_cache.shader_id));
			
			// set mvp
			glUniformMatrix4fv(shader_cache.param_mvp, 1, GL_FALSE, item.mvp.data());

			// draw elements
			glBindVertexArray(static_cast<GLuint>(mesh_cache.vao));
			glDrawElements(GL_TRIANGLES, mesh_cache.vert_count, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
}

Id add_mesh(Mesh&& mesh, Id shader)
{
	return global_mesh_store.add_mesh(std::move(mesh), shader);
}

static Render3dType render_mesh(const Frame& frame, Id elem_id, const Mat44& transform)
{
	const Id mesh_id = get_elem_attr(frame, elem_id, attrs::mesh_id);

	const auto& mesh_cache = global_mesh_store.mesh_cache(mesh_id);
	const auto& shader_cache = global_mesh_store.shader_cache(mesh_id);

	// use shader
	glUseProgram(static_cast<GLuint>(shader_cache.shader_id));
	
	// set mvp
	const auto& mesh_tf = get_elem_attr(frame, elem_id, attrs::transform);
	const auto& mvp = transform * mesh_tf;
	glUniformMatrix4fv(shader_cache.param_mvp, 1, GL_FALSE, mvp.data());
 
	// draw elements
	glBindVertexArray(static_cast<GLuint>(mesh_cache.vao));
	glDrawElements(GL_TRIANGLES, mesh_cache.vert_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	return Render3dType::single;
}

static const Id mesh_elem_type = register_elem_type([](ElementTypeSetup& setup)
{
	setup.set_name("mesh");
	setup.set_attr(attrs::renderer_3d, &render_mesh);
});

namespace elem
{
	Id mesh(const Context ctx)
	{
		return make_element(ctx, mesh_elem_type);
	}
}

