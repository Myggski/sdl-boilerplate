# sdl-boilerplate

## Overview

Welcome to my Game Development Toolbox! This project serves as my personal toolkit for crafting games with SDL2 using CMake and MinGW. The goal here? Efficiency and learning. Instead of reinventing the wheel for every game, I'm building a collection of reusable features and leveling up my skills in C++, CMake, and Visual Studio Code along the way.

Regarding the persistent presence of the .vscode folder – it's intentionally sticking around. Setting up Visual Studio Code using C++ can be a head-scratcher in my opinion, and finding the right setup isn't always a cakewalk. Even if it's not perfect, this folder is here to get the job done. Let's call it a functional work in progress.

## Purpose
This toolkit isn't just about storing code snippets; it's a strategy. By reusing proven features, I streamline game development. Think of it as a Swiss Army knife for game creators, ensuring a smoother and more enjoyable development process.

## Getting Started 

1. git clone https://github.com/your-username/your-toolbox-project.git
2. Make sure to have [MinGW](https://www.mingw-w64.org/) and [CMake](https://cmake.org/download/) installed
3. Make sure that the paths to mingw64 in .vscode/c_cpp_properties.json, .vscode/launch.json and .vscode/settings.json are correct
4. Replace "sdl-boilerplate" inside CMakeLists.txt and tasks.json to your project name
5. Add environment variables for SDL2DIR, SDL2IMAGEDIR, and SDL2TTFDIR, with the paths where they are installed. This will hopefully change into one 'SDK' folder instead, if possible, in the future.

Hopefully you will have it up and running offering Build Debug x64 and Build Debug x86 options. Stay tuned for two additional choices: Build Release x64 and Build Release x86.


