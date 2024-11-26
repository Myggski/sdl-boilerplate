#define SDL_MAIN_HANDLED

#include "GameEngine.h"
#include <SDL.h>
#include <stdexcept>
#include <chrono>
#include "src/EventManager.h"
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

    SDLQuitEventId = Engine::EventManager::GetInstance().RegisterEventListener(SDL_QUIT, [&](const SDL_Event &Event)
                                                                               { Shutdown(); });

    if (EngineData != nullptr)
    {
      EngineData->Initialize(Window, Renderer);
    }

    return true;
  }

  void GameEngine::Update()
  {
    constexpr float FixedTimeStep{1.0f / 60.0f}; // 60 FPS
    constexpr uint8_t MaxFrameSkip{5};           // Limit to prevent spiraling
    float Lag{0.0f};

    std::chrono::steady_clock::time_point PreviousTime{std::chrono::high_resolution_clock::now()};

    while (IsGameRunning)
    {
      // Calculate delta time
      std::chrono::steady_clock::time_point CurrentTime{std::chrono::high_resolution_clock::now()};
      std::chrono::duration<float> ElapsedTime{CurrentTime - PreviousTime};
      float DeltaTime{ElapsedTime.count()};
      PreviousTime = CurrentTime;

      // Accumulate time to handle fixed-step updates
      Lag += DeltaTime;

      // Process input
      EventManager &EventManager{EventManager::GetInstance()};
      EventManager.PollEvents();

      if (!IsGameRunning)
      {
        break;
      }

      // Fixed update loop
      uint16_t UpdateCount{0};
      while (Lag >= FixedTimeStep && UpdateCount < MaxFrameSkip)
      {
        if (EngineData)
        {
          EngineData->Update(FixedTimeStep);
        }
        Lag -= FixedTimeStep;
        ++UpdateCount;
      }

      // Render only once per frame
      if (EngineData)
      {
        EngineData->Draw(Renderer);
      }

      // Remove
      EventManager.SafelyRemovePendingEvents();

      // Update the screen
      SDL_RenderClear(Renderer);
      SDL_GL_SwapWindow(Window);
    }

    // Destroy window and quit SDL
    SDL_DestroyWindow(Window);
    SDL_DestroyRenderer(Renderer);
    SDL_Quit();
  }

  void GameEngine::Shutdown()
  {
    IsGameRunning = false;

    if (EngineData)
    {
      EngineData->Shutdown();
    }

    Engine::EventManager::GetInstance().RemoveEventListener(SDL_QUIT, SDLQuitEventId);
  }
}