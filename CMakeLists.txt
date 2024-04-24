cmake_minimum_required(VERSION 3.24)
project(MyProject)

# Enable the generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")

# Include ftphysics to build the shared library
add_subdirectory(PhysicsEngine)

# Include ftgraphics to build the executable
add_subdirectory(GraphicsViewer)

# Add a dependency to ensure ftGraphics doesn't build until ftPhysics is built
add_dependencies(ftGraphics ftPhysics) # Ensures ftPhysics builds first
