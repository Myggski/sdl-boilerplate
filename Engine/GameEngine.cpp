#define SDL_MAIN_HANDLED

#include "GameEngine.h"
#include <SDL.h>
#include <stdexcept>

namespace Engine
{
  void GameEngine::Run()
  {
    if (!Initialize())
    {
      return;
    }

    Update();
  }

  bool GameEngine::Initialize()
  {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return false;
    }

    Window = SDL_CreateWindow("SDL2 Boilerplate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (!Window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      return false;
    }

    IsGameRunning = true;

    if (EngineData != nullptr)
    {
      EngineData->Initialize();
    }

    return true;
  }

  void GameEngine::Update()
  {
    SDL_Event Event;

    while (IsGameRunning)
    {
      // Handle events
      while (SDL_PollEvent(&Event) != 0)
      {
        if (Event.type == SDL_QUIT)
        {
          Shutdown();
        }
      }

      if (EngineData)
      {
        EngineData->Update(0.f);
      }

      // Update the screen
      SDL_GL_SwapWindow(Window);
    }

    // Destroy window
    SDL_DestroyWindow(Window);

    // Quit SDL
    SDL_Quit();
  }

  void GameEngine::Shutdown()
  {
    IsGameRunning = false;

    if (EngineData)
    {
      EngineData->Shutdown();
    }
  }
}