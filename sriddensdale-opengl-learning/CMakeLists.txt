cmake_minimum_required(VERSION 3.25)

project(sriddensdale-opengl-learning
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put all generated CMake files in the build directory.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
include(cmake/dependencies.cmake)

add_subdirectory(core)

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        core
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${glm_SOURCE_DIR}
)
