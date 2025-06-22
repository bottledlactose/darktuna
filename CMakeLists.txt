cmake_minimum_required(VERSION 3.18)
project(darktuna VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_SHARED OFF CACHE BOOL "Build shared SDL3 library")
set(SDL_STATIC ON CACHE BOOL "Build static SDL3 library")

set(IMGUI_SOURCES 
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    # This backend doesn't force us to compile shaders in some funky way...
    thirdparty/imgui/backends/imgui_impl_sdl3.cpp
    thirdparty/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

add_subdirectory(thirdparty/sdl)

set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "Build static PortAudio library")
add_subdirectory(thirdparty/portaudio)

add_executable(darktuna
    source/main.cpp
    source/App.hpp
    source/App.cpp
    source/Tuner.hpp
    source/Tuner.cpp
    ${IMGUI_SOURCES})

target_include_directories(darktuna PRIVATE
    data
    thirdparty/sdl/include
    thirdparty/imgui
    thirdparty/portaudio/include
)
target_link_libraries(darktuna PRIVATE SDL3-static portaudio)

# Disable console window on release builds
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(darktuna PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()
