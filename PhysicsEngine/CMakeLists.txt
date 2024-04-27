cmake_minimum_required(VERSION 3.9)

project(
  ftPhysics
  VERSION 1.0.1
  DESCRIPTION "My cyclon physics engine emplimentation")

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)
add_compile_options(-Wall -Werror -Wextra -pg -O3)

add_link_options(-lGL -lGLEW -ldl -lpthread -pg -O3)

cmake_minimum_required(VERSION 3.10)
project(ftPhysics)

# Create the shared library
set(PHYSICS_SOURCES
    src/ft_forceGenerator.cpp
    src/ft_pcontacts.cpp
    src/ft_plinks.cpp
    src/ft_pworld.cpp
    src/ft_pForceGenerator.cpp
    src/ft_contacts.cpp
    src/ft_joint.cpp
    src/ft_body.cpp
    src/ft_collideCoarse.cpp
    src/ft_collideFine.cpp
    src/ft_world.cpp)

add_library(ftPhysics SHARED ${PHYSICS_SOURCES})

# Define the export target
install(
  TARGETS ftPhysics
  EXPORT ftPhysicsTargets
  LIBRARY DESTINATION ../install/lib) # Local installation

# Install the header files
set(PHYSICS_HEADERS
    ftPhysics.h
    includes/ft_particle.h
    includes/ft_pcontacts.h
    includes/ft_contacts.h
    includes/ft_def.h
    includes/ft_threads.h
    includes/ft_forceGenerator.h
    includes/ft_pForceGenerator.h
    includes/ft_plinks.h
    includes/ft_pworld.h
    includes/ft_joint.h
    includes/ft_body.h
    includes/ft_world.h
    includes/ft_collideCoarse.h
    includes/ft_collideFine.h)

install(FILES ${PHYSICS_HEADERS} DESTINATION ../install/include/ftPhysics)

# Install the package configuration file
install(
  EXPORT ftPhysicsTargets
  FILE ftPhysicsConfig.cmake
  DESTINATION ../install/share/cmake/ftPhysics) # Local installation
