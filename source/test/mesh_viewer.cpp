
#include "mesh_viewer.h"
#include "engine/math_utils.h"
#include "engine/viewport.h"

int main()
{
	MeshViewerApp app;
	while (!app.ended())
	{
		app.update();
	}
	return 0;
}

MeshViewerApp::MeshViewerApp()
{
	WindowCreationParams params;
	params.width = 1024;
	params.height = 768;
	params.title = "Mesh Viewer";
	window = Window::create(params);

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
	store.mesh_shader = create_shader(desc);

	const auto id = add_mesh(std::move(mesh), store.mesh_shader);


	// create a item
	model.cam_pose = make_lookat(Vec3{4, 3, 3}, Vec3{0, 0, 0}, Vec3{0, 1, 0});
	model.meshes = { { id, Pose{} } };

	start_time = std::chrono::system_clock::now();

	presenter.set_present_object(this);
}

void MeshViewerApp::update()
{
	presenter.process_control(window->poll_events());

	// rotate the mesh item
	auto& item = model.meshes[0];
	item.pose.rot = 
		make_quat_angle_axis(0.01f, Vec3{0, 1, 0}) * 
		item.pose.rot *
		make_quat_angle_axis(0.01f, Vec3{1, 0, 0});

	// render stuff out
	// renderer->render(store, model);

	const double dt = 1.0 / 60.0;
	presenter.step_frame(dt);

	// present
	window->present();

	frame_count++;
	auto now = std::chrono::system_clock::now();
	if ((now - start_time) >= std::chrono::duration<double>(1.0))
	{
		printf("-- fps: %f\n", frame_count / (std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time).count()));
		start_time = now;
		frame_count = 0;
	}
}

bool MeshViewerApp::ended()
{
	return window->should_close();
}

void MeshViewerApp::present(const Context& ctx)
{
	using namespace elem;

	Viewpoint vp;
	vp.projection = make_perspective(to_rad(60.f), window->aspect(), 0.1f, 100.f);
	vp.pose = model.cam_pose;	

	viewport(_ctx);
	_attr(attrs::width, static_cast<double>(window->width()));
	_attr(attrs::height, static_cast<double>(window->height()));
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		for (size_t i = 0; i < model.meshes.size(); i++)
		{
			auto& mesh_model = model.meshes[i];
			mesh(_ctx_id(i));
			_attr(attrs::mesh_id, mesh_model.mesh_id);
			_attr(attrs::transform, to_mat44(mesh_model.pose));
		}
	}
}

