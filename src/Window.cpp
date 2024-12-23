#include "Window.h"

std::unique_ptr<Window> Window::createWindow() {
    std::unique_ptr<Window> window = std::unique_ptr<Window>(new Window());
    window->initWindow();
    return window;
}


void Window::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}


void Window::initWindow() {
    glfwInit();

    // glfw로 창관리만 하고 렌더링 API는 안 쓴다는 설정
    // 렌더링 및 그래픽 처리는 외부 API인 Vulkan으로 함
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
    // window에 현재 App 객체를 바인딩
    glfwSetWindowUserPointer(window, this);
    // 프레임버퍼 사이즈 변경 콜백 함수 등록
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}


void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {

}