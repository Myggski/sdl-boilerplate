#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "engine/GameEvent.h"

class MyClass
{
public:
    void MemberFunction(int x)
    {
        std::cout << "MemberFunction called with: " << x << std::endl;
    }

    void AnotherMemberFunction(float f, bool flag)
    {
        std::cout << "AnotherMemberFunction called with: " << f << ", " << flag << std::endl;
    }
};

int main(int argc, char *argv[])
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("SDL2 Boilerplate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Main loop
    bool quit = false;
    SDL_Event e;

    GameEvent<int> event;
    auto lambda = [](int val)
    { std::cout << "Lambda: " << val << std::endl; };

    while (!quit)
    {
        // Handle events
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // Your rendering code goes here

        // Update the screen
        SDL_GL_SwapWindow(window);
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    return 0;
}