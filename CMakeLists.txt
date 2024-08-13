cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/share)

set(PROJECT_NAME VoxelEngine)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

project (${PROJECT_NAME})

include_directories(include include/imgui)
add_executable (${PROJECT_NAME} ${SOURCES})

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(png REQUIRED)

target_include_directories (${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL GLEW::GLEW png)