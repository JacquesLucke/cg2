cmake_minimum_required(VERSION 3.5)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-source builds are not allowed")
endif()

project(cg2)
set (CMAKE_CXX_STANDARD 14)


macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRC ${SRCS} PARENT_SCOPE)
    endif()
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