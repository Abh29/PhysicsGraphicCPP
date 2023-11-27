cmake_minimum_required(VERSION 3.24)
project(ftGraphics)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra)
#set(CMAKE_CXX_FLAGS " -lEGL -lGLU -lOpenGL ")
#add_link_options( -lSDL2 -lGL -lSDL2_image )
#add_link_options( -lglfw -lGL -lGLEW -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)

## add glsl files compilation
find_program(GLSLC glslc)
find_program(GLSL_VALIDATOR glslangValidator)

file(GLOB_RECURSE GLSL_SOURCE_FILES
#    add the shader files here
        )

foreach(GLSL_F ${GLSL_SOURCE_FILES})
    GET_FILENAME_COMPONENT(FILE_NAME ${GLSL_F} NAME)
    SET(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    ADD_CUSTOM_COMMAND(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders"
            COMMAND ${GLSLC} ${GLSL_F} -o ${SPIRV}
            DEPENDS ${GLSL_F}
            VERBATIM
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL_F)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

# copy texture folder to build
file(COPY textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(Texture)
#add_custom_command(
#        TARGET Texture
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
#)

# copy model folder to build
file(COPY models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(Models)
#add_custom_command(
#        TARGET Models
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
#)



add_link_options( -lglfw -lGL -lGLEW -lvulkan -ldl -lpthread )
add_executable(ftGraphics main.cpp includes/ft_headers.h)

add_dependencies(ftGraphics Shaders)
