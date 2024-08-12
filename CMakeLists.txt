cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_NAME VoxelEngine)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.lib)

project (${PROJECT_NAME})

include_directories(include include/imgui)
add_executable (${PROJECT_NAME} ${SOURCES})

target_include_directories (${PROJECT_NAME} PRIVATE lib/include)
target_link_libraries(${PROJECT_NAME} opengl32.lib ${LIBRARIES})