#include "GameEngine.h"
#include <SDL.h>
#include <stdexcept>
#include <chrono>
#include <SDL_image.h>

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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
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

    Context = std::make_unique<EngineContext>(Window, Renderer);

    SDLQuitEventId = Context->Dispatcher.RegisterEventListener(SDL_QUIT, [&](const SDL_Event &Event)
                                                                { Shutdown(); });

    IsGameRunning = true;

    if (EngineData != nullptr)
    {
      EngineData->Startup(*Context);
    }

    return true;
  }

  void GameEngine::Update()
  {
    constexpr float FixedTimeStep = 1.0f / 60.0f;
    constexpr uint8_t MaxFrameSkip = 5;
    float Lag = 0.0f;

    Engine::Camera &MainCamera = Context->MainCamera;

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

      Context->Dispatcher.PollEvents();

      if (!IsGameRunning)
      {
        break;
      }

      // UI layout/hit-testing happen here, before gameplay reads input, so this frame's pointer
      // claim (see InputManager::SetPointerClaimed) is already set by the time Game::Update()
      // asks, not one frame late.
      Context->UICanvas.UpdateLayout(Renderer);
      Context->UICanvas.ProcessInput(Context->Input);

      // Fixed update loop
      uint8_t UpdateCount{0};
      while (Lag >= FixedTimeStep && UpdateCount < MaxFrameSkip)
      {
        Context->World.RunSystems(FixedTimeStep);
        if (EngineData)
        {
          EngineData->Update(*Context, FixedTimeStep);
        }
        Lag -= FixedTimeStep;
        ++UpdateCount;
      }

      MainCamera.PreRender();

      // Render only once per frame
      Context->Overlay.BeginFrame();
      if (EngineData)
      {
        EngineData->Draw(*Context);
      }

      // UI (and the debug overlay below) render in real screen pixels, not the pixel-art zoom
      // scale the game world just rendered at.
      MainCamera.ResetScale();
      Context->UICanvas.Render(Renderer);
      Context->Overlay.EndFrame(Renderer, MainCamera);

      MainCamera.PostRender();

      // Update the screen
      SDL_RenderClear(Renderer);
      SDL_GL_SwapWindow(Window);

      Context->Input.LateUpdate();
    }

    Cleanup();
  }

  void GameEngine::Shutdown()
  {
    IsGameRunning = false;

    if (EngineData && Context)
    {
      EngineData->Shutdown(*Context);
    }
  }

  void GameEngine::Cleanup()
  {
    if (Context)
    {
      Context->Dispatcher.RemoveEventListener(SDL_QUIT, SDLQuitEventId);
    }

    // Destroy Assets/Input/Camera/Overlay/Dispatcher while the renderer and window are still
    // valid, since AssetManager frees SDL_Texture objects that belong to the renderer and
    // DebugOverlay tears down Dear ImGui's SDL/renderer backends.
    Context.reset();

    SDL_DestroyWindow(Window);
    SDL_DestroyRenderer(Renderer);
    SDL_Quit();
  }
}
