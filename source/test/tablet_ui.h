#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"

class TabletUIApp
{
public:
	TabletUIApp();
	void update();
	bool ended();
	void present(const Context& ctx);

private:
	Id tablet_shader;
	Id tablet_screen_shader;
	Texture atlas_texture;
	int click_count{};
};