#pragma once

#include "Common.h"

class Window {
public:
	static std::unique_ptr<Window> createWindow();
	~Window() { }
	void cleanup();


	GLFWwindow* getWindow() { return window; }

private:
	Window() {}

	GLFWwindow* window;

	void initWindow();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};