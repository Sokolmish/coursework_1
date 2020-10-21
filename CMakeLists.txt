cmake_minimum_required(VERSION 3.10)
project(WatVis)
project(WatVis VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Enable O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()

add_compile_options(-Wall)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
 
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"

    "${PROJECT_SOURCE_DIR}/include/*/*.h"
    "${PROJECT_SOURCE_DIR}/include/*/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*/*.c"
)

include_directories(
    ${PROJECT_SOURCE_DIR}/headers
    ${PROJECT_SOURCE_DIR}/src
)

add_executable(WatVis ${all_SRCS})
target_link_libraries (WatVis 
    ${OPENGL_LIBRARIES} 
    ${GLEW_LIBRARIES} 
    glfw ${GLFW_LIBRARIES} 
    ${FREETYPE_LIBRARIES}
)
