
#include "test/mesh_viewer.h"

int main()
{
	MeshViewerApp app;

	while (!app.ended())
	{
		app.update();
	}
	return 0;
}
