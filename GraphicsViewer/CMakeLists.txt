cmake_minimum_required(VERSION 3.24)
project(
  ftGraphics
  VERSION 1.0.1
  DESCRIPTION "My backend renderer based on vulkan")

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)
add_compile_options(-Wall -Werror -Wextra -pg -O3)


# add glsl files compilation
find_program(GLSLC glslc)
find_program(GLSL_VALIDATOR glslangValidator)

# add the shader files here
file(
  GLOB_RECURSE
  GLSL_SOURCE_FILES
  shaders/shader.vert
  shaders/shader.frag
  shaders/MinimalRdrSys.vert
  shaders/MinimalRdrSys.frag
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


# Set the CMAKE_PREFIX_PATH to point to the correct install directory
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../install")


# Create a shared library 
set(GRAPHICS_SOURCES
  src/ft_attachment.cpp
  src/ft_buffer.cpp
  src/ft_callbacks.cpp
  src/ft_camera.cpp
  src/ft_command.cpp
  src/ft_descriptor.cpp
  src/ft_device.cpp
  src/ft_event.cpp
  src/ft_frameBuffer.cpp
  src/ft_image.cpp
  src/ft_instance.cpp
  src/ft_model.cpp
  src/ft_parser.cpp
  src/ft_physicalDevice.cpp
  src/ft_picker.cpp
  src/ft_pipeline.cpp
  src/ft_renderPass.cpp
  src/ft_renderer.cpp
  src/ft_rendering_systems.cpp
  src/ft_sampler.cpp
  src/ft_scene.cpp
  src/ft_shader.cpp
  src/ft_surface.cpp
  src/ft_swapChain.cpp
  src/ft_texture.cpp
  src/ft_vertex.cpp
  src/ft_window.cpp
  )

add_library(ftGraphics SHARED ${GRAPHICS_SOURCES})
add_dependencies(ftGraphics Shaders)


# Define the export target
install(
  TARGETS ftGraphics
  EXPORT ftGraphicsTargets
  LIBRARY DESTINATION ../install/lib) # Local installation


set(SHADER_DIR "${PROJECT_BINARY_DIR}/shaders/")
add_definitions(-DSHADER_DIR=${SHADER_DIR})

# Install the header files
set(GRAPHICS_HEADERS
  includes/ftGraphics.h
  includes/ft_attachment.h
  includes/ft_buffer.h
  includes/ft_callbacks.h
  includes/ft_camera.h
  includes/ft_command.h
  includes/ft_component.h
  includes/ft_defines.h
  includes/ft_descriptor.h
  includes/ft_device.h
  includes/ft_event.h
  includes/ft_frameBuffer.h
  includes/ft_headers.h
  includes/ft_image.h
  includes/ft_instance.h
  includes/ft_model.h
  includes/ft_parser.h
  includes/ft_physicalDevice.h
  includes/ft_picker.h
  includes/ft_pipeline.h
  includes/ft_renderPass.h
  includes/ft_renderer.h
  includes/ft_rendering_systems.h
  includes/ft_sampler.h
  includes/ft_scene.h
  includes/ft_sceneObject.h
  includes/ft_shader.h
  includes/ft_surface.h
  includes/ft_swapChain.h
  includes/ft_texture.h
  includes/ft_threads.h
  includes/ft_tools.h
  includes/ft_vertex.h
  includes/ft_window.h
  includes/stb_image.h
)

install(FILES ${GRAPHICS_HEADERS} DESTINATION ../install/include/ftGraphics)


# Install the package configuration file
install(
  EXPORT ftGraphicsTargets
  FILE ftGraphicsConfig.cmake
  DESTINATION ../install/share/cmake/ftGraphics) # Local installation




