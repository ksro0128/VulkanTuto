#include "App.h"

void App::run() {
    window = Window::createWindow();
    renderer = Renderer::createRenderer(window->getWindow());
    scene = Scene::createScene();
    renderer->loadScene(scene.get());

    mainLoop();
    cleanup();
}


/*
    렌더링 루프 실행	
*/
void App::mainLoop() {
    while (!glfwWindowShouldClose(window->getWindow())) {
        glfwPollEvents();
        renderer->drawFrame(scene.get());
    }
    vkDeviceWaitIdle(renderer->getDevice());  // 종료시 실행 중인 GPU 작업을 전부 기다림
}


void App::cleanup() {
    scene->cleanup();
    renderer->cleanup();
    window->cleanup();
}
