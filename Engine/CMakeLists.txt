cmake_minimum_required(VERSION 3.16)
project(sdl-boilerplate-engine)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

# Set the sources for the Engine
file(GLOB ENGINE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ECS/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SDL/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameEngine.cpp
)

set(ENGINE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GameEngine.h
)

file(TO_CMAKE_PATH "$ENV{CMAKE_PREFIX_PATH}" CMAKE_SDK_PATH) # converts a native <path> into a cmake-style path with forward-slashes
set(FMOD_DIR "${CMAKE_SDK_PATH}/FMOD")

# Paths for FMOD
set(FMOD_INCLUDE_DIR "${FMOD_DIR}/include")
set(FMOD_LIB_DIR "${FMOD_DIR}/lib/${ARCH}")
set(FMOD_LIB "${FMOD_LIB_DIR}/fmod_vc.lib") # Windows dynamic library for FMOD

# Create the shared library
add_library(${PROJECT_NAME} SHARED ${ENGINE_SOURCE} ${ENGINE_HEADERS})

# Set properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR} # This ensures the DLL goes directly to build/${ARCH}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR} # This ensures the .lib/.dll.a goes to the same directory
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_BUILD_DLL PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${CMAKE_SOURCE_DIR}/Engine/
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${FMOD_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${FMOD_LIB}
)

target_precompile_headers(${PROJECT_NAME} PUBLIC PrecompiledHeader.h)

# List of DLLs to copy
set(DLLS_TO_COPY
    ${SDL2_INCLUDE_DIRS}/../lib/${ARCH}/SDL2.dll
    ${SDL2_IMAGE_INCLUDE_DIRS}/../lib/${ARCH}/SDL2_image.dll
    ${SDL2_TTF_INCLUDE_DIRS}/../lib/${ARCH}/SDL2_ttf.dll
    ${FMOD_LIB_DIR}/fmod.dll
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
message(STATUS "FMOD Include Directory: ${FMOD_INCLUDE_DIR}")
message(STATUS "FMOD Library Directory: ${FMOD_LIB_DIR}")
message(STATUS "FMOD Library: ${FMOD_LIB}")
