cmake_minimum_required(VERSION 3.5)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-source builds are not allowed")
endif()

project(cg2)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(extern)
add_subdirectory(source)

SET(SRC
    source/main.cc
    source/timer.cc
    extern/glad/src/glad.c
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui.cpp
    extern/imgui/imgui_impl_glfw_gl3.cpp
)

set(INC
    extern/glfw/include
    extern/glad/include
    extern/imgui
)

set(LIB glfw)

find_package(OpenGL REQUIRED)
list(APPEND LIB ${OPENGL_LIBRARIES})

add_executable(cg2 ${SRC})
include_directories(${INC})
target_link_libraries(cg2 ${LIB})