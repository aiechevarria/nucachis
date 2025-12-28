project(NuCachis)

cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories(include)
include_directories(lib/CLI11)
include_directories(lib/imgui)
include_directories(lib/imgui/backends)
include_directories(lib/ImGuiFileDialog)
include_directories(lib/parsers)

# NuCachis project source files
set(PROJECT_SOURCES
    src/Misc.cpp
    src/Logo.cpp
    src/MemoryElement.cpp
    src/MainMemory.cpp
    src/Cache.cpp
    src/Simulator.cpp
    src/ParserConfig.cpp
    src/ParserTrace.cpp
    src/GUI.cpp
    src/Main.cpp
)

# ImGui source files
set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_sdl2.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

# ImGuiFileDialog source files
set(IMGUIFILEDIALOG_SOURCES
    lib/ImGuiFileDialog/ImGuiFileDialog.cpp
)

# Parsers
set(PARSER_SOURCES
    lib/parsers/dictionary.c
    lib/parsers/iniparser.c
)

# Combine both
add_executable(NuCachis
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUIFILEDIALOG_SOURCES}
    ${PARSER_SOURCES}
)

# Link SDL2 and OpenGL
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

target_link_libraries(NuCachis
    SDL2::SDL2
    OpenGL::GL
)
