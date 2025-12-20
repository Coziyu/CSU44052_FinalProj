cmake_minimum_required(VERSION 3.0)
project(imaginative_wonderland)

find_package(OpenGL REQUIRED)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clangd
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF) # Required to avoid linker errors
add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	external/tinygltf-2.9.3/
	src/
)

add_executable(wonderland
	src/main.cpp
)
target_link_libraries(wonderland
	${OPENGL_LIBRARY}
	glfw
	glad
)
