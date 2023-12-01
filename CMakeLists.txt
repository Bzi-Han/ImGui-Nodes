cmake_minimum_required(VERSION 3.13.0)

project(ImGuiNodes)

option(IMGUI_NODES_BUILD_TESTING "Build test programs." ON)

set(CMAKE_CXX_STANDARD 20)

include_directories(
    .
    third_party
    third_party/imgui
    third_party/imgui/examples/libs/glfw/include
)

# Build libImGuiNodes
add_library(${PROJECT_NAME} STATIC modules/ImGuiNodes.cc)

# Build test program
if (IMGUI_NODES_BUILD_TESTING)
    file(GLOB_RECURSE COMMON_SOURCE_FILES src/common/*.cc)

    # Scan imgui sources
    aux_source_directory(third_party/imgui IMGUI_SOURCES)
    # Make imgui backends sources
    set(
        IMGUI_BACKENDS_SOURCES
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_opengl3.cpp
    )

    add_executable(test src/test/main.cc ${COMMON_SOURCE_FILES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS_SOURCES})
    target_link_options(test PRIVATE /NODEFAULTLIB:MSVCRT)
    target_link_directories(test PRIVATE third_party/imgui/examples/libs/glfw/lib-vc2010-64)
    target_link_libraries(test glfw3 opengl32 legacy_stdio_definitions ${PROJECT_NAME})
endif()
