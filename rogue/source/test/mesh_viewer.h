
#pragma once

#include "math/math_types.h"
#include "types/id.h"
#include "graphics/mesh.h"
#include "graphics/viewpoint.h"
#include "window/window.h"
#include <vector>
#include <memory>

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
	MeshStore mesh_store;
};

class MeshViewerRenderer
{
public:
	MeshViewerRenderer(Window& window);
	void render(const MeshViewerStore& store, const MeshViewerModel& model);

private:
	Viewpoint vp;
	MeshDrawStream stream;
};

class MeshViewerApp
{
public:
	MeshViewerApp();
	void update();
	bool ended();

private:
	std::unique_ptr<Window> window;
	std::unique_ptr<MeshViewerRenderer> renderer;

	MeshViewerModel model;
	MeshViewerStore store;
};