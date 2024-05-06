cmake_minimum_required(VERSION 3.24)
project(ftGraphics)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra -pg -O3)

# Imgui
add_subdirectory(imgui)

# add glsl files compilation
find_program(GLSLC glslc)
find_program(GLSL_VALIDATOR glslangValidator)

file(
  GLOB_RECURSE
  GLSL_SOURCE_FILES
  # add the shader files here
  shaders/shader.vert
  shaders/shader.frag
  shaders/SimpleRdrSys.vert
  shaders/SimpleRdrSys.frag
  shaders/InstanceRdrSys.vert
  shaders/InstanceRdrSys.frag
  shaders/OneTextureRdrSys.vert
  shaders/OneTextureRdrSys.frag
  shaders/TwoTextureRdrSys.vert
  shaders/TwoTextureRdrSys.frag
  shaders/Picker.vert
  shaders/Picker.frag
  shaders/SkyBoxRdrSys.frag
  shaders/SkyBoxRdrSys.vert
  shaders/OutlineRdrSys.vert
  shaders/OutlineRdrSys.frag
  shaders/NormDebugRdrSys.vert
  shaders/NormDebugRdrSys.frag
  shaders/NormDebugRdrSys.geom)

set(GLSLC_FLAGS -g -Werror)

foreach(GLSL_F ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL_F} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders"
    COMMAND ${GLSLC} ${GLSLC_FLAGS} ${GLSL_F} -o ${SPIRV}
    DEPENDS ${GLSL_F}
    VERBATIM)
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL_F)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

# copy texture folder to build file(COPY textures DESTINATION
# ${CMAKE_CURRENT_BINARY_DIR}) add_custom_target(Texture)

# copy model folder to build
file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(Assets)

# include_directories(external/ktx/other_include) set(KTX_DIR
# ${CMAKE_CURRENT_SOURCE_DIR}/external/ktx) set(KTX_SOURCES
# ${KTX_DIR}/lib/texture.c ${KTX_DIR}/lib/hashlist.c
# ${KTX_DIR}/lib/checkheader.c ${KTX_DIR}/lib/swap.c ${KTX_DIR}/lib/memstream.c
# ${KTX_DIR}/lib/filestream.c)
#
# add_library(ktx STATIC ${KTX_SOURCE})

add_link_options(
  -lglfw
  -lGL
  -lGLEW
  -lvulkan
  -ldl
  -lpthread
  -lktx
  -pg
  -O3)

find_package(nlohmann_json REQUIRED)

# Set the CMAKE_PREFIX_PATH to point to the correct install directory
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../install")

# Find the shared library package configuration
find_package(ftPhysics REQUIRED)

# Create an executable
add_executable(
  ftGraphics
  main.cpp
  src/ft_window.cpp
  src/ft_app.cpp
  src/ft_callbacks.cpp
  src/ft_shader.cpp
  src/ft_instance.cpp
  src/ft_physicalDevice.cpp
  src/ft_device.cpp
  src/ft_surface.cpp
  src/ft_swapChain.cpp
  src/ft_image.cpp
  src/ft_buffer.cpp
  src/ft_frameBuffer.cpp
  src/ft_vertex.cpp
  src/ft_command.cpp
  src/ft_attachment.cpp
  src/ft_renderPass.cpp
  src/ft_event.cpp
  src/ft_gui.cpp
  src/ft_model.cpp
  src/ft_scene.cpp
  src/ft_renderer.cpp
  src/ft_pipeline.cpp
  src/ft_rendering_systems.cpp
  src/ft_descriptor.cpp
  src/ft_camera.cpp
  src/ft_sampler.cpp
  src/ft_texture.cpp
  src/ft_picker.cpp
  src/ft_parser.cpp
  includes/ft_tools.h
  includes/ft_threads.h)

target_link_libraries(ftGraphics imgui ftPhysics nlohmann_json::nlohmann_json)
add_dependencies(ftGraphics Shaders ftPhysics)

# Include headers from ftPhysics for #include statements
target_include_directories(ftGraphics
                           PRIVATE "${CMAKE_PREFIX_PATH}/include/ftPhysics")
