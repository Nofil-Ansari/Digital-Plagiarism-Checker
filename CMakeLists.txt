cmake_minimum_required(VERSION 3.16)
project(DigitalPlagiarismChecker C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/glfw)

# Dear ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# CLI Target
add_executable(PlagiarismCheckerCLI
    Digital_Plagiarism_checker.cpp
    PlagiarismCore.hpp
)

# GUI Target
add_executable(PlagiarismCheckerGUI
    gui_main.cpp
    PlagiarismCore.hpp
    ${IMGUI_SOURCES}
)

target_include_directories(PlagiarismCheckerGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    third_party/glfw/include
)

target_link_libraries(PlagiarismCheckerGUI PRIVATE
    glfw
    opengl32
    gdi32
    comdlg32
)
