# sdl-boilerplate

## Overview

Welcome to my SDL Toolbox! This project is a personal toolkit for crafting games with SDL2 using CMake and LLVM/Clang. It aims to enhance efficiency and learning by building reusable features and improving skills in modern C++, CMake, and Visual Studio Code.

## Purpose

This toolkit is a personal project aimed at simplifying game development by reusing effective features that I find useful. It's intended to be a practical tool, enhancing the development experience and growing in usefulness as I create more games.

## Prerequisites

Before building the project, ensure you have the following installed:

- **CMake**: Version 3.15 or higher. Download and install from [cmake.org](https://cmake.org/download/).
- **LLVM/Clang**: Required for compiling with Clang on Windows. Install LLVM/Clang from [llvm.org](https://llvm.org/releases/download.html).
- **Ninja**: Build system. Download and extract Ninja from [github.com/ninja-build/ninja/releases](https://github.com/ninja-build/ninja/releases).
- **SDL2**: Required libraries. You may need to download SDL2 development libraries and adjust paths accordingly in your CMake configuration.

## Build Instructions

Currently, I only have Debug setup, but I hope to have Release prepared as well quite soon.

### Configure and Build (Debug, x64/x86)

Open a terminal and execute the following commands:

```bash
# Navigate to your project directory
cd path/to/your/project

# Example 1: Configure and build for x64 architecture
cmake -S . -B build/x64 -G Ninja -DARCH=x64
cmake --build build/x64 --config Debug --target sdl-boilerplate

# Example 2: Configure and build for x86 architecture
cmake -S . -B build/x86 -G Ninja -DARCH=x86
cmake --build build/x86 --config Debug --target sdl-boilerplate
```

> [!IMPORTANT]
> Replace 'path/to/your/project' with the actual path to your project directory. Adjust paths and configurations as needed based on your environment setup and SDL2 installation.

## Environment variables

To ensure proper functionality, you need to set up the following environment variables:

### CMAKE_PREFIX_PATH 

Path to your SDK folder containing necessary projects like SDL2, SDL2_ttf, and SDL2_image. 
> [!TIP]
> Example: C:\Code\sdks

### PATH 

Add LLVM bin directory to PATH.
> [!TIP]
> Example: C:\Program Files\LLVM\bin