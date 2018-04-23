cmake_minimum_required(VERSION 3.5)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-source builds are not allowed")
endif()

project(cg2)
set(CMAKE_CXX_STANDARD 14)

macro(add_source fileName)
    file(RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    list(APPEND _SRC "${_relPath}/${fileName}")
    set(SRC ${_SRC} PARENT_SCOPE)
endmacro()

add_subdirectory(extern)
add_subdirectory(source)

set(INC extern/glfw/include)

set(LIB glfw)

find_package(OpenGL REQUIRED)
list(APPEND LIB ${OPENGL_LIBRARIES})

add_executable(cg2 ${SRC})
include_directories(${INC})
target_link_libraries(cg2 ${LIB})