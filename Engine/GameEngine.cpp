#define SDL_MAIN_HANDLED

#include "GameEngine.h"
#include <SDL.h>
#include <stdexcept>
#include "src/InputManager.h"

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

    Window = SDL_CreateWindow("SDL2 Boilerplate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800, SDL_WINDOW_SHOWN);
    if (!Window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      return false;
    }

    Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (Renderer == nullptr)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
      return false;
    }

    IsGameRunning = true;
    InputManager::CreateInstance();

    if (EngineData != nullptr)
    {
      EngineData->Initialize(Window, Renderer);
    }

    return true;
  }

  void GameEngine::Update()
  {
    while (IsGameRunning)
    {
      InputManager::GetInstance().Pull();

      if (EngineData)
      {
        EngineData->Update(0.f);
        EngineData->Draw(Renderer);
      }

      // Update the screen
      SDL_RenderClear(Renderer);
      SDL_GL_SwapWindow(Window);
    }

    // Destroy window and quit SDL
    SDL_DestroyWindow(Window);
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