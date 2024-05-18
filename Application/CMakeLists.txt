cmake_minimum_required(VERSION 3.24)
project(
  ftApp 
  VERSION 1.0.1
  DESCRIPTION "My Application based on imgui, using the GraphicsViwer and PhysicsEngine
  developed separately")

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wextra -pg -O3)

# Imgui
add_subdirectory(imgui)

# copy model folder to build
file(COPY assets misc DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
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
find_package(ftGraphics REQUIRED)


set(MISC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/misc/")
add_definitions(-DMISC_DIR=${MISC_DIR})


# Create an executable
add_executable(
  ftApp 
  main.cpp
  src/ft_app.cpp
  src/ft_gui.cpp
  src/ft_physicsApp.cpp
  src/ft_rigidObject.cpp
  src/ft_rigidComponent.cpp
  src/ft_jsonParser.cpp
  )

target_link_libraries(ftApp imgui ftGraphics ftPhysics nlohmann_json::nlohmann_json)
add_dependencies(ftApp ftGraphics ftPhysics)

# Include headers from ftPhysics for #include statements
target_include_directories(ftApp PRIVATE "${CMAKE_PREFIX_PATH}/include/ftPhysics")
target_include_directories(ftApp PRIVATE "${CMAKE_PREFIX_PATH}/include/ftGraphics")

