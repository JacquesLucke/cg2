cmake_minimum_required(VERSION 3.5)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-source builds are not allowed")
endif()

project(cg2)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(extern)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

SET(EX_SRC
    extern/glad/src/glad.c
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui.cpp
    extern/imgui/imgui_impl_glfw_gl3.cpp
)

file(GLOB_RECURSE SOURCE "source/intern/*.cpp")

include_directories(
    extern/glfw/include/
    extern/glad/include/
    extern/glm/
    extern/imgui/
    extern/Eigen/
    extern/sparsepp/
    source/
)

# copy resource files
file(COPY "resources" DESTINATION ".")

set(LIB glfw)

find_package(OpenGL REQUIRED)
list(APPEND LIB ${OPENGL_LIBRARIES})

# compile applications
file(GLOB apps "source/apps/*.cpp")
foreach(app ${apps})
  string(REGEX REPLACE "[^.]*/([^.]+)\\.[^/]*" "\\1" APP_NAME ${app})

  add_executable(${APP_NAME} ${SOURCE} ${EX_SRC} ${app})
  target_link_libraries(${APP_NAME} ${LIB})
endforeach()
