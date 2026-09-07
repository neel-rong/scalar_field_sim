cmake_minimum_required(VERSION 3.20)

project(ScalarFieldSim LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()


# =========================================================
# Dependencies
# =========================================================

find_package(OpenGL REQUIRED)

add_subdirectory(third_party/glfw)


add_library(glad
    third_party/glad/src/glad.c
)

target_include_directories(glad
    PUBLIC
        third_party/glad/include
)


add_library(imgui
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
)

target_link_libraries(imgui
    PUBLIC
        glfw
        glad
        OpenGL::GL
)


# =========================================================
# Simulation Core
# =========================================================

add_library(ScalarFieldSimCore
    "src/simulation_operators.cpp"
)

target_include_directories(ScalarFieldSimCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# =========================================================
# Main Application
# =========================================================

add_executable(ScalarFieldSim
    src/main.cpp
    src/simulation.cpp
    src/renderer.cpp
    src/GUI.cpp
)

target_link_libraries(ScalarFieldSim
    PRIVATE
        ScalarFieldSimCore
        OpenGL::GL
        glfw
        glad
        imgui
)


# =========================================================
# Tests
# =========================================================

add_executable(ScalarFieldSimTests
    simtest/test_main.cpp
    simtest/sim_test.cpp
)

target_include_directories(ScalarFieldSimTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/simtest
)

target_link_libraries(ScalarFieldSimTests
    PRIVATE
        ScalarFieldSimCore
)


add_test(
    NAME ScalarFieldSimulationTest
    COMMAND ScalarFieldSimTests
)


# =========================================================
# Post Build
# =========================================================

add_custom_command(TARGET ScalarFieldSim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:ScalarFieldSim>/shaders
)