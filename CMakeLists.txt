cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(PROJECT_NAME VoxelEngine)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


project (${PROJECT_NAME})
add_executable (${PROJECT_NAME} ${SOURCES})


target_include_directories (${PROJECT_NAME} PRIVATE include/glfw include/glew)

target_link_libraries(${PROJECT_NAME} opengl32.lib ${PROJECT_SOURCE_DIR}/lib/glew32s.lib ${PROJECT_SOURCE_DIR}/lib/glfw3.lib)