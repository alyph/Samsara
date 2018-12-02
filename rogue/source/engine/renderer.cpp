#include "renderer.h"
#include "color.h"
#include <GL/glew.h>

namespace renderer
{
	void clear(const Color& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}	
}