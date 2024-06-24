cmake_minimum_required(VERSION 3.16)

# Specify the C and C++ compilers
set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang++.exe" CACHE STRING "CPP Compiler" FORCE)
set(CMAKE_C_COMPILER "C:/Program Files/LLVM/bin/clang.exe" CACHE STRING "C Compiler" FORCE)
set(CMAKE_RC_COMPILER "C:/Program Files/LLVM/bin/llvm-rc.exe" CACHE STRING "RC Compiler" FORCE)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(sdl-boilerplate)

# Set the build type
set(CMAKE_BUILD_TYPE Debug)

# Other CMake Modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Set architecture
if((NOT ARCH AND ${CMAKE_SIZEOF_VOID_P} MATCHES 8))
    set(ARCH "x64")
elseif(NOT ARCH)
    set(ARCH "x86")
endif()

# Set the build and install directories based on the architecture
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build/${ARCH}" CACHE STRING "BuildDir" FORCE)
set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/install/${ARCH}" CACHE STRING "InstallDir" FORCE)

# Add subdirectories for engine and game projects
add_subdirectory(Engine)
add_subdirectory(Game)
