﻿cmake_minimum_required(VERSION 3.15)
project(sdl-boilerplate)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Set the build type if not defined
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Check if the size of pointers is 4 (32-bit) or 8 (64-bit)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH "x86")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
else()
    message(FATAL_ERROR "Unsupported architecture")
endif()

# Set the build directory based on the architecture
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build/${ARCH}")

# Set the installation directory based on the architecture
set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/install/${ARCH}")

# Find SDL2 things
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set the output directory for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIR}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARY_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

# Set the build type
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Set the build and install directories
set(CMAKE_BINARY_DIR ${BUILD_DIR})
set(EXECUTABLE_OUTPUT_PATH ${BUILD_DIR})
set(LIBRARY_OUTPUT_PATH ${BUILD_DIR})
set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install prefix" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${ARCH})

# Print information about the chosen architecture
message(STATUS "Selected architecture: ${ARCH}")
message(STATUS "Build directory: ${BUILD_DIR}")
message(STATUS "Install directory: ${INSTALL_DIR}")

# Copy assets to the build directory during build
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${BUILD_DIR})

# Copy required DLL files to the build directory during build (adjust the paths accordingly)
file(COPY
    ${SDL2_INCLUDE_DIRS}/../lib/${ARCH}/SDL2.dll
    ${SDL2_IMAGE_INCLUDE_DIRS}/../lib/${ARCH}/SDL2_image.dll
    ${SDL2_TTF_INCLUDE_DIRS}/../lib/${ARCH}/SDL2_ttf.dll # Fix the DLL path
    DESTINATION ${BUILD_DIR}
)
