cmake_minimum_required(VERSION 3.10)
project(imaginative_wonderland)

find_package(OpenGL REQUIRED)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clangd
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF) # Required to avoid linker errors
add_subdirectory(external)

# include_directories(
# 	# external/glfw-3.1.2/include/
# 	external/glfw-3.3/include/
# 	external/glm-0.9.7.1/
# 	external/glad-opengl-3.3/include/
# 	external/tinygltf-2.9.3/
# 	external/
# 	src/
# )



add_executable(wonderland
src/core/Shader.cpp
src/core/Camera.cpp
src/core/Texture.cpp
src/core/ModelEntity.cpp
src/models/Phoenix.cpp
src/models/ArchTree.cpp
src/tinygltf_impl.cpp
src/Entities.cpp
src/Window.cpp
src/ResourceManager.cpp
src/utils.cpp
src/Terrain.cpp
src/Perlin.cpp
src/main.cpp
)

target_include_directories(wonderland
    PRIVATE
        src/
        src/core/
        src/models/
)
target_include_directories(wonderland
    SYSTEM PRIVATE
        external/glfw-3.3/include/
        external/glm-0.9.7.1/
        external/glad-opengl-3.3/include/
        external/tinygltf-2.9.3/
        external/json/
        external/
)

target_link_libraries(wonderland
	${OPENGL_LIBRARY}
	glfw
	glad
)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(wonderland 
    imgui::imgui
)

find_package(OpenMP REQUIRED)
if(OpenMp_CXX_FOUND)
    target_link_libraries(wonderland OpenMP::OpenMP_CXX)
    message("OpenMP found")
else()
    message("Finding OpenMP manually, this is hardcoded to work on my machine unfortunately.")
    target_compile_options(wonderland PRIVATE -fopenmp)
    target_link_libraries(wonderland "D:/Program Files/LLVM/lib/libomp.lib")
endif()



