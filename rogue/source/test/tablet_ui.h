#pragma once

class TabletUIApp
{
public:
	TabletUIApp();
	void update();
	bool ended();
	void present(const Context& ctx);
};