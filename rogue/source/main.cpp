
#include "test/mesh_viewer.h"
#include "test/tablet_test.h"

int main()
{
	// MeshViewerApp app;
	TabletTestApp app;

	while (!app.ended())
	{
		app.update();
	}
	return 0;
}
