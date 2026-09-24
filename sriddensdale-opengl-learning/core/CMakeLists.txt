add_library(core STATIC
        mathf.h
        external/glad.cpp
)

target_include_directories(core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core
        PUBLIC
        glfw
        IMGUI
)

target_include_directories(core
        PUBLIC
        ${glm_SOURCE_DIR}
)
