cmake_minimum_required(VERSION 3.24)
project(ftPhysics)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra -pg -O3)
# copy texture folder to build file(COPY textures DESTINATION
# ${CMAKE_CURRENT_BINARY_DIR}) add_custom_target(Texture)

add_link_options(-lGL -lGLEW -ldl -lpthread -pg -O3)

add_executable(ftPhysics main.cpp src/ft_particle.cpp src/ft_pcontacts.cpp
                         includes/ft_threads.h)
