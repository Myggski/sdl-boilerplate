#define SDL_MAIN_HANDLED

#include "GameEngine.h"
#include <SDL.h>
#include <stdexcept>
#include <chrono>
#include "src/SDL/SDLEventDispatcher.h"
#include "src/SDL/SDLEventHandler.h"
#include "src/AssetManager.h"
#include "src/InputManager.h"
#include <SDL_image.h>
#include "src/Camera.h"

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

    Window = SDL_CreateWindow("SDL2 Boilerplate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_SHOWN);
    if (!Window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      return false;
    }

    SDL_RaiseWindow(Window);

    Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (Renderer == nullptr)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
      return false;
    }

    SDLQuitEventId = Dispatcher.RegisterEventListener(SDL_QUIT, [&](const SDL_Event &Event)
                                                      { Shutdown(); });

    // Initialize managers
    Engine::AssetManager::GetInstance().Initialize(Renderer);
    Engine::InputManager::GetInstance().Initialize(Dispatcher);
    Engine::Camera::Initialize(Window, Renderer, 320, 180, 6.f); // Initialize camera with default values

    IsGameRunning = true;

    if (EngineData != nullptr)
    {
      EngineData->Initialize(Window, Renderer, &Dispatcher);
    }

    return true;
  }

  void GameEngine::Update()
  {
    constexpr float FixedTimeStep = 1.0f / 60.0f;
    constexpr uint8_t MaxFrameSkip = 5;
    float Lag = 0.0f;

    Engine::Camera &MainCamera = Engine::Camera::GetMainCamera();

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

      Dispatcher.PollEvents();

      if (!IsGameRunning)
      {
        break;
      }

      // Fixed update loop
      uint8_t UpdateCount{0};
      while (Lag >= FixedTimeStep && UpdateCount < MaxFrameSkip)
      {
        if (EngineData)
        {
          EngineData->Update(FixedTimeStep);
        }
        Lag -= FixedTimeStep;
        ++UpdateCount;
      }

      MainCamera.PreRender();

      // Render only once per frame
      if (EngineData)
      {
        EngineData->Draw(Renderer);
      }

      MainCamera.PostRender();

      // Update the screen
      SDL_RenderClear(Renderer);
      SDL_GL_SwapWindow(Window);
    }

    Cleanup();
  }

  void GameEngine::Shutdown()
  {
    IsGameRunning = false;

    if (EngineData)
    {
      EngineData->Shutdown();
    }
  }

  void GameEngine::Cleanup()
  {
    Dispatcher.RemoveEventListener(SDL_QUIT, SDLQuitEventId);
    InputManager::GetInstance().Clear();

    SDL_DestroyWindow(Window);
    SDL_DestroyRenderer(Renderer);
    SDL_Quit();
  }
}