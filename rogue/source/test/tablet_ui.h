#pragma once
#include "engine/window.h"
#include "engine/presenter.h"
#include <vector>
#include <memory>
#include <string>

class TabletUIApp
{
public:
	TabletUIApp();
	void update();
	bool ended();

private:
	void present(Context ctx);

	std::unique_ptr<Window> window;
	Presenter presenter;
};