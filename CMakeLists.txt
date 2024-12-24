cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME VULKAN_TUTORIAL)
set(CMAKE_CXX_STANDARD 17)
 
set(WINDOW_NAME "VULKAN_TUTORIAL")
set(WINDOW_WIDTH 1920)
set(WINDOW_HEIGHT 1080)

project(${PROJECT_NAME})
set(SRC src/main.cpp
        src/Common.h src/Common.cpp
        src/VulkanUtil.cpp src/VulkanUtil.h
        src/Window.cpp src/Window.h
        src/VulkanContext.cpp src/VulkanContext.h
        src/Buffer.cpp src/Buffer.h
        src/Mesh.cpp src/Mesh.h
        src/Model.cpp src/Model.h
        src/Texture.cpp src/Texture.h
        src/Object.cpp src/Object.h
        src/Scene.cpp src/Scene.h
        src/SwapChain.cpp src/SwapChain.h
        src/SyncObjects.cpp src/SyncObjects.h
        src/CommandBuffers.cpp src/CommandBuffers.h
        src/RenderPass.cpp src/RenderPass.h
        src/FrameBuffers.cpp src/FrameBuffers.h
        src/DescriptorSetLayout.cpp src/DescriptorSetLayout.h
        src/ShaderResourceManager.cpp src/ShaderResourceManager.h
        src/Pipeline.cpp src/Pipeline.h
        src/Renderer.cpp src/Renderer.h
        src/App.cpp src/App.h)

add_executable(${PROJECT_NAME} ${SRC})

include(Dependency.cmake)

# 우리 프로젝트에 include / lib 관련 옵션 추가


set(CMAKE_PREFIX_PATH "C:/VulkanSDK/1.3.296.0")
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)

target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

target_compile_definitions(${PROJECT_NAME} PUBLIC
WINDOW_NAME="${WINDOW_NAME}"
WINDOW_WIDTH=${WINDOW_WIDTH}
WINDOW_HEIGHT=${WINDOW_HEIGHT})

# Dependency들이 먼저 build 될 수 있게 관계 설정 / 뒤에서 부터 컴파일
add_dependencies(${PROJECT_NAME} ${DEP_LIST})

add_compile_options(/utf-8)