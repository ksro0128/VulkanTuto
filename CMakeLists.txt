# 최소 CMake 버전 설정
cmake_minimum_required(VERSION 3.15)
project(VulkanTuto)

# C++17 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan SDK 설정
find_package(Vulkan REQUIRED)

# GLFW 설정
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only")
add_subdirectory(external/glfw)

# GLM 설정
add_subdirectory(external/glm)

# 실행 파일 생성
add_executable(VulkanApp src/main.cpp)

# include 디렉토리 추가
target_include_directories(VulkanApp PRIVATE
    ${Vulkan_INCLUDE_DIR}
    external/glfw/include
    external/glm
)

# 라이브러리 연결
target_link_libraries(VulkanApp PRIVATE Vulkan::Vulkan glfw glm)
