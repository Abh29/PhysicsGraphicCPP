cmake_minimum_required(VERSION 3.24)
project(MyProject)

# Enable the generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")

# Include ftphysics to build the shared library
add_subdirectory(PhysicsEngine)

# Include ftgraphics to build the shared library 
add_subdirectory(GraphicsViewer)

# Include ftApp to build the executable
add_subdirectory(Application)

# Add a dependency to ensure ftApp doesn't build until both ftGraphics and ftPhysics are built
add_dependencies(ftApp ftGraphics ftPhysics)

