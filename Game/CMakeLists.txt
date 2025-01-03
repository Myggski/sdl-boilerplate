cmake_minimum_required(VERSION 3.16)
project(sdl-boilerplate-game)

# Dear ImGui
file(TO_CMAKE_PATH "$ENV{CMAKE_PREFIX_PATH}" CMAKE_SDK_PATH) # converts a native <path> into a cmake-style path with forward-slashes
set(IMGUI_DIR "${CMAKE_SDK_PATH}/imgui")

include_directories(${IMGUI_DIR} "${IMGUI_DIR}/backends")

# Collect ImGui source files
file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp"
)

# Set source files
set(SOURCE_FILES
    ${IMGUI_SOURCES} # Directly use IMGUI_SOURCES
    Game.cpp
    ${CMAKE_SOURCE_DIR}/Engine/EntryPoint.h # Include EntryPoint.h if it contains main()
)

# Copy the entire assets folder and its contents (subdirectories and files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Set compile options for the target (if needed)
target_compile_options(${PROJECT_NAME} PRIVATE -g)

# Compile definitions and include directories
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_DIR=" ${PROJECT_SOURCE_DIR}/${PROJECT_NAME} "
    " _CRT_SECURE_NO_WARNINGS "
)

# Include directories (ensure to include engine headers)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine
    ${IMGUI_DIR}
    "${IMGUI_DIR}/backends" # Use forward slashes for path consistency
)

# Link against Engine library (sdl-boilerplate-engine.dll)
target_link_libraries(${PROJECT_NAME} PRIVATE
    sdl-boilerplate-engine
)

# Optionally set the output executable name
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    OUTPUT_NAME ${CMAKE_PROJECT_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Set the startup project for Visual Studio (if needed)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})