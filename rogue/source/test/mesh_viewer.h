
#pragma once

#include "engine/math_types.h"
#include "engine/id.h"
#include "engine/mesh.h"
#include "engine/viewpoint.h"
#include "engine/shader.h"
#include "engine/window.h"
#include "engine/presenter.h"
#include <vector>
#include <memory>
#include <chrono>

class MeshViewerModel
{
public:
	struct MeshItem
	{
		Id mesh_id;
		Pose pose;
	};

	Pose cam_pose;
	std::vector<MeshItem> meshes;

};

class MeshViewerStore
{
public:
	Id mesh_shader;
	MeshStore mesh_store;
};

class MeshViewerApp
{
public:
	MeshViewerApp();
	void update();
	bool ended();
	void present(const Context& ctx);

private:
	std::unique_ptr<Window> window;
	Presenter presenter;

	MeshViewerModel model;
	MeshViewerStore store;

	std::chrono::system_clock::time_point start_time;
	int frame_count{};
};