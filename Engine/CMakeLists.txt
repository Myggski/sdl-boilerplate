cmake_minimum_required(VERSION 3.16)
project(sdl-boilerplate-engine)

# SDL2 / SDL2_image / SDL2_ttf are fetched automatically (see cmake/Dependencies.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

# Engine-owned assets (currently just the default UI font, see AssetManager::LoadDefaultFont).
# Copied into the same shared ${CMAKE_BINARY_DIR}/assets Game/CMakeLists.txt copies its own
# assets/ into, so both land in one merged assets/ folder next to the built executable.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Set the sources for the Engine
file(GLOB ENGINE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ecs/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ecs/component/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ecs/system/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameEngine.cpp
)

set(ENGINE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameEngine.h
)

# Dear ImGui debug overlay (see Engine/src/DebugOverlay.h), Debug builds only. Not fetched via
# FetchContent since ImGui has no official CMake package; expected under CMAKE_PREFIX_PATH like
# SDL2 used to be. Compiled into the Engine DLL rather than the Game exe so a future Editor
# target can share the exact same compiled ImGui (ImGui keeps process-wide global state, so it
# can only safely exist as one compiled copy shared across the DLL boundary, see
# WINDOWS_EXPORT_ALL_SYMBOLS below).
file(TO_CMAKE_PATH "$ENV{CMAKE_PREFIX_PATH}" CMAKE_SDK_PATH)
set(IMGUI_DIR "${CMAKE_SDK_PATH}/imgui")

file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp"
)

# Create the shared library
add_library(${PROJECT_NAME} SHARED ${ENGINE_SOURCE} ${ENGINE_HEADERS})

target_sources(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:${IMGUI_SOURCES}>)
set_source_files_properties(${IMGUI_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

# Set properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR} # This ensures the DLL goes directly to build/${ARCH}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR} # This ensures the .lib/.dll.a goes to the same directory
    # ImGui (and anything else without explicit dllexport annotations) needs this to be callable
    # from outside the DLL at all; Engine's own classes already export explicitly via ENGINE_API,
    # this just additionally covers third-party code that doesn't.
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_BUILD_DLL PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# EntryPoint.h provides its own main(). SDL_MAIN_HANDLED stops <SDL.h> from redefining main to
# SDL_main, which would otherwise happen the moment any translation unit (Engine's or Game's)
# includes <SDL.h>. PUBLIC so it applies to both this DLL and anything linking against it.
target_compile_definitions(${PROJECT_NAME} PUBLIC SDL_MAIN_HANDLED)

# Lets both Engine's own code (DebugOverlay.cpp) and any consumer (Game, a future Editor) guard
# ImGui:: calls with #ifdef ENGINE_WITH_DEBUG_UI. PUBLIC so consumers see the same definition
# Engine was actually built with.
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:ENGINE_WITH_DEBUG_UI>)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${CMAKE_SOURCE_DIR}/Engine/
    $<$<CONFIG:Debug>:${IMGUI_DIR}>
    $<$<CONFIG:Debug>:${IMGUI_DIR}/backends>
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)

target_precompile_headers(${PROJECT_NAME} PUBLIC PrecompiledHeader.h)

# List of DLLs to copy next to the built executable
set(DLLS_TO_COPY
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE:SDL2_image::SDL2_image>
    $<TARGET_FILE:SDL2_ttf::SDL2_ttf>
)

# Copy each DLL using a loop
foreach(DLL ${DLLS_TO_COPY})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# Print architecture information
message(STATUS "Target Include Directory: ${CMAKE_SOURCE_DIR}/Engine/src")
message(STATUS "Selected architecture: ${ARCH}")
message(STATUS "Build directory: ${BUILD_DIR}")
message(STATUS "Install directory: ${INSTALL_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler type: ${CMAKE_CXX_COMPILER}")
