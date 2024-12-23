#pragma once

#include "Common.h"
#include "Window.h"
#include "Renderer.h"
#include "Scene.h"

class App {
public:
	void run();
    App() {};
    ~App() {};

private:
	std::unique_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Scene> scene;

	void mainLoop();
	void cleanup();
};
