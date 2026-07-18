cmake_minimum_required(VERSION 3.16)
project(sdl-boilerplate-game)

# Set source files
set(SOURCE_FILES
    Game.cpp
    framework/Bootstrap.cpp
    ${CMAKE_SOURCE_DIR}/Engine/EntryPoint.h # Include EntryPoint.h if it contains main()
)

# Copy the entire assets folder and its contents (subdirectories and files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Debug symbols in every build type, not just Debug. Needs -g on both the compile step (embeds
# debug info in each .obj) and the link step (tells lld-link to actually assemble and emit a .pdb
# from it): one without the other silently produces no .pdb at all.
target_compile_options(${PROJECT_NAME} PRIVATE -g)
target_link_options(${PROJECT_NAME} PRIVATE -g)

# Compile definitions and include directories
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_DIR=" ${PROJECT_SOURCE_DIR}/${PROJECT_NAME} "
    " _CRT_SECURE_NO_WARNINGS "
)

# Include directories (ensure to include engine headers)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine
)

# Link against Engine library (sdl-boilerplate-engine.dll). Dear ImGui's headers/include dirs and
# the ENGINE_WITH_DEBUG_UI compile definition (Debug builds only) come along publicly from Engine.
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

# `cmake --install` support: RUNTIME only (the .exe), not the .pdb. See top-level CMakeLists.txt
# for where CMAKE_INSTALL_PREFIX (install/${ARCH}) gets set, and for the shared assets/ install.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
