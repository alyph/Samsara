
#include "mesh_viewer.h"
#include "graphics/renderer.h"
#include "math/math_utils.h"

MeshViewerApp::MeshViewerApp()
{
	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = "Mesh Viewer";
	window = Window::create(params);

	renderer = std::make_unique<MeshViewerRenderer>(*window);

	// create a mesh (cube), left handed
	Mesh mesh;
	mesh.indices = 
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
	};

	mesh.vertices = 
	{
		// bottom
		{ -1.f, -1.f, -1.f },
		{ -1.f, -1.f, 1.f },
		{ 1.f, -1.f, 1.f },
		{ 1.f, -1.f, -1.f },

		// top
		{ -1.f, 1.f, -1.f },
		{ 1.f, 1.f, -1.f },
		{ 1.f, 1.f, 1.f },
		{ -1.f, 1.f, 1.f },

		// back
		{ -1.f, -1.f, -1.f },
		{ 1.f, -1.f, -1.f },
		{ 1.f, 1.f, -1.f },
		{ -1.f, 1.f, -1.f },

		// front
		{ -1.f, -1.f, 1.f },
		{ -1.f, 1.f, 1.f },
		{ 1.f, 1.f, 1.f },
		{ 1.f, -1.f, 1.f },

		// left
		{ -1.f, -1.f, -1.f },
		{ -1.f, 1.f, -1.f },
		{ -1.f, 1.f, 1.f },
		{ -1.f, -1.f, 1.f },

		// right
		{ 1.f, -1.f, -1.f },
		{ 1.f, -1.f, 1.f },
		{ 1.f, 1.f, 1.f },
		{ 1.f, 1.f, -1.f },
	};

	mesh.colors = 
	{
		// bottom
		{ 1.f, 1.f, 0.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f },

		// top
		{ 1.f, 0.f, 0.f, 1.f },
		{ 1.f, 0.f, 0.f, 1.f },
		{ 1.f, 0.f, 0.f, 1.f },
		{ 1.f, 0.f, 0.f, 1.f },

		// back
		{ 0.f, 1.f, 1.f, 1.f },
		{ 0.f, 1.f, 1.f, 1.f },
		{ 0.f, 1.f, 1.f, 1.f },
		{ 0.f, 1.f, 1.f, 1.f },

		// front
		{ 0.f, 1.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },

		// left
		{ 1.f, 0.f, 1.f, 1.f },
		{ 1.f, 0.f, 1.f, 1.f },
		{ 1.f, 0.f, 1.f, 1.f },
		{ 1.f, 0.f, 1.f, 1.f },

		// right
		{ 0.f, 0.f, 1.f, 1.f },
		{ 0.f, 0.f, 1.f, 1.f },
		{ 0.f, 0.f, 1.f, 1.f },
		{ 0.f, 0.f, 1.f, 1.f },
	};

	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/test/simple_vs.gls";
	desc.fs_path = "../../data/test/simple_fs.gls";
	auto shader = Shader::create(desc);

	const auto id = store.mesh_store.add_mesh(std::move(mesh), shader);


	// create a item
	model.cam_pose = make_lookat(Vec3{4, 3, 3}, Vec3{0, 0, 0}, Vec3{0, 1, 0});
	model.meshes = { { id.value(), Pose{} } };
}

void MeshViewerApp::update()
{
	window->poll_events();

	// rotate the mesh item
	auto& item = model.meshes[0];
	item.pose.rot = 
		make_quat_angle_axis(0.01f, Vec3{0, 1, 0}) * 
		item.pose.rot *
		make_quat_angle_axis(0.01f, Vec3{1, 0, 0});

	// render stuff out
	renderer->render(store, model);

	// present
	window->present();
}

bool MeshViewerApp::ended()
{
	return window->should_close();
}

MeshViewerRenderer::MeshViewerRenderer(Window& window)
{
	// TODO: window asepct may change at runtime.
	vp.projection = make_perspective(to_rad(60.f), window.aspect(), 0.1f, 100.f);
}

void MeshViewerRenderer::render(const MeshViewerStore& store, const MeshViewerModel& model)
{
	vp.pose = model.cam_pose;
	const auto mat_vp = calc_mat_vp(vp);

	stream.items.resize(model.meshes.size());
	for (size_t i = 0; i < model.meshes.size(); i++)
	{
		auto& mesh_model = model.meshes[i];
		auto& item = stream.items[i];
		item.mesh_id = mesh_model.mesh_id;
		item.mvp = (mat_vp * to_mat44(mesh_model.pose));
	}

	// clear
	renderer::clear(Color{});

	// draw meshes
	renderer::draw(store.mesh_store, stream);
}