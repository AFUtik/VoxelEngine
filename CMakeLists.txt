option(VOXELENGINE_BUILD_WINDOWS_VCPKG ON)
if(VOXELENGINE_BUILD_WINDOWS_VCPKG AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

cmake_minimum_required (VERSION 3.8)



set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/share)

set(PROJECT_NAME VoxelEngine)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

project (${PROJECT_NAME})

include_directories(include include/imgui)
add_executable (${PROJECT_NAME} ${SOURCES})

target_compile_options(VoxelEngine PRIVATE /W0)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL GLEW::GLEW ZLIB::ZLIB PNG::PNG glm::glm)