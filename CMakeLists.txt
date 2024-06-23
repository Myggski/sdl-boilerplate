cmake_minimum_required(VERSION 3.15)

# Specify the C and C++ compilers
set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang++.exe" CACHE STRING "CPP Compiler" FORCE) # Path to clang-compiler
set(CMAKE_C_COMPILER "C:/Program Files/LLVM/bin/clang.exe" CACHE STRING "C Compiler" FORCE) # Path to clang-compiler
set(CMAKE_RC_COMPILER "C:/Program Files/LLVM/bin/llvm-rc.exe" CACHE STRING "RC Compiler" FORCE) # Path to the resource compiler

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type, will fix for release later
set(CMAKE_BUILD_TYPE Debug)

# The project
project(sdl-boilerplate)

# Other CMake Modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Set architecture
if((NOT ARCH AND ${CMAKE_SIZEOF_VOID_P} MATCHES 8))
    set(ARCH "x64")
elseif(NOT ARCH)
    set(ARCH "x86")
endif()

# Set the build and install directories based on the architecture
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build/${ARCH}")
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

# Set compile options for the target
target_compile_options(${PROJECT_NAME} PRIVATE -g)

if(ARCH STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

# Set the build and install directories
set(EXECUTABLE_OUTPUT_PATH ${BUILD_DIR})
set(LIBRARY_OUTPUT_PATH ${BUILD_DIR})
set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install prefix" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${ARCH})

# Set the output directory for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIR}
)

# Copy assets to the build directory during build
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${BUILD_DIR})

# Copy required DLL files to the build directory during build (adjust the paths accordingly)
file(COPY
    ${SDL2_INCLUDE_DIRS}/../lib/${ARCH}/SDL2.dll
    ${SDL2_IMAGE_INCLUDE_DIRS}/../lib/${ARCH}/SDL2_image.dll
    ${SDL2_TTF_INCLUDE_DIRS}/../lib/${ARCH}/SDL2_ttf.dll # Fix the DLL path
    DESTINATION ${BUILD_DIR}
)

# Print information about the chosen architecture
message(STATUS "Selected architecture: ${ARCH}")
message(STATUS "Build directory: ${BUILD_DIR}")
message(STATUS "Install directory: ${INSTALL_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler type: ${CMAKE_CXX_COMPILER}")