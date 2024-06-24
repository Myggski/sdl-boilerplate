cmake_minimum_required(VERSION 3.15)

project(sdl-boilerplate-engine)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

file(GLOB HEADER_FILES
    ${CMAKE_SOURCE_DIR}/Engine/src/*.h
    ${CMAKE_SOURCE_DIR}/Engine/EntryPoint.h
    ${CMAKE_SOURCE_DIR}/Engine/GameEngine.h
)

# Source files
set(SOURCE_FILES
    ${HEADER_FILES}
    ${CMAKE_SOURCE_DIR}/Engine/GameEngine.cpp
)

# Create the shared library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Set properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR} # This ensures the DLL goes directly to build/${ARCH}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR} # This ensures the .lib/.dll.a goes to the same directory
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_BUILD_DLL PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}/")

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE PrecompiledHeader.h)

# Copy required DLL files #TEMPORARY
file(COPY
    ${SDL2_INCLUDE_DIRS}/../lib/${ARCH}/SDL2.dll
    DESTINATION ${BUILD_DIR}
)

# Print architecture information
message(STATUS "Selected architecture: ${ARCH}")
message(STATUS "Build directory: ${BUILD_DIR}")
message(STATUS "Install directory: ${INSTALL_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler type: ${CMAKE_CXX_COMPILER}")