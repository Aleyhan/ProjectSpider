cmake_minimum_required(VERSION 3.16)
project(ProjectSpider LANGUAGES C CXX OBJC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
        GLEW_STATIC
        GL_SILENCE_DEPRECATION
)

# for the headers, glew and Angel and glfw
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/glew/include
        ${CMAKE_SOURCE_DIR}/external/Angel/include
        ${CMAKE_SOURCE_DIR}/include/obstacle
        ${CMAKE_SOURCE_DIR}/external/include/tinyobjloader
)

# Build platform libraries
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

# Build GLEW as a static library
add_library(glew STATIC
        ${CMAKE_SOURCE_DIR}/external/glew/src/glew.c
)

target_include_directories(glew PUBLIC
        ${CMAKE_SOURCE_DIR}/external/glew/include
)

# build Angel as a static library
add_library(angel_shaders STATIC
        ${CMAKE_SOURCE_DIR}/external/Angel/src/InitShader.cpp
)
target_include_directories(angel_shaders PUBLIC
        ${CMAKE_SOURCE_DIR}/external/Angel/include
)

#  my own executable files
add_executable(ProjectSpider
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/Abdomen.cpp
        ${CMAKE_SOURCE_DIR}/src/camera/Camera.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/Axes.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/Spider.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/Cephalothorax.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/PerlinNoise.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/Head.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/Eye.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/LegSegment.cpp
        ${CMAKE_SOURCE_DIR}/src/spider/Leg.cpp
        ${CMAKE_SOURCE_DIR}/include/obstacle/Obstacle.cpp
        ${CMAKE_SOURCE_DIR}/include/model/Model.cpp
        ${CMAKE_SOURCE_DIR}/external/tinyobjloader/tiny_obj_loader.cc
)

# link the libraries to the executable
target_link_libraries(ProjectSpider PRIVATE
        glfw
        glew
        angel_shaders
        ${PLATFORM_LIBS}
)

target_link_libraries(ProjectSpider PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework OpenGL"
        "-framework GLUT"
)

# Create shaders directory in build output
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

# Copy all shaders into build/shaders before each build
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/shaders/*")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(NAME ${SHADER} NAME)
    add_custom_command(
            TARGET ProjectSpider PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SHADER}"
            "${CMAKE_BINARY_DIR}/shaders/${NAME}"
            COMMENT "Copying shader ${NAME} to shaders/"
    )
endforeach()

# set the output directory for the executable
set_target_properties(ProjectSpider PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)