cmake_minimum_required(VERSION 3.5)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-source builds are not allowed")
endif()

project(cg2)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(extern)

SET(EX_SRC
    extern/glad/src/glad.c
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui.cpp
    extern/imgui/imgui_impl_glfw_gl3.cpp
)

file(GLOB_RECURSE SOURCE "source/*.cpp")

include_directories(
    extern/glfw/include/
    extern/glad/include/
    extern/imgui/
    include/
)

set(LIB glfw)

find_package(OpenGL REQUIRED)
list(APPEND LIB ${OPENGL_LIBRARIES})

add_executable(cg2 ${SOURCE} ${EX_SRC})
target_link_libraries(cg2 ${LIB})
