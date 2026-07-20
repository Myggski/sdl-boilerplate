#include "GameEngine.h"
#include "RPC.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <chrono>
#include <deque>
#include <SDL3_image/SDL_image.h>

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
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return false;
    }

    // Windows are shown by default now (no more SDL_WINDOW_SHOWN flag, no x/y position args).
    Window = SDL_CreateWindow("SDL3 Boilerplate", 1920, 1080, 0);
    if (!Window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      return false;
    }

    SDL_RaiseWindow(Window);

    // No more index/flags params; vsync (was SDL_RENDERER_PRESENTVSYNC) is now a separate call.
    Renderer = SDL_CreateRenderer(Window, nullptr);
    if (Renderer == nullptr)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
      return false;
    }
    SDL_SetRenderVSync(Renderer, 1);

    Context = std::make_unique<EngineContext>(Window, Renderer);

    SDLQuitEventId = Context->Dispatcher.RegisterEventListener(SDL_EVENT_QUIT, [&](const SDL_Event &Event)
                                                                { Shutdown(); });

    IsGameRunning = true;

    if (EngineData != nullptr && !EngineData->Startup(*Context))
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game::Startup failed, aborting.\n");
      return false;
    }

    return true;
  }

  void GameEngine::Update()
  {
    constexpr float FixedTimeStep = 1.0f / 60.0f;
    constexpr uint8_t MaxFrameSkip = 5;
    float Lag = 0.0f;

    // Buffered once per real frame (below, right after Network.Poll()), drained once per
    // fixed-step tick (in the loop below) - same real-frame-buffer/fixed-step-drain split
    // PendingMoveCommands uses for input, for the same reason: a discrete event read at real-frame
    // cadence could be missed by a fixed-step Update() that runs zero times that frame.
    std::deque<ConnectionEvent> PendingConnectionEvents;
    std::deque<NetworkMessage> PendingNetworkMessages;

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

      // Once per real frame, before anything below reads network state - PreUpdate's own comment
      // calls out "a remote peer's commands feeding the same queue" as local input does, so this
      // frame's messages need to already be available by the time PreUpdate runs.
      Context->Network.Poll();

      // Buffer only - not fired/dispatched until the fixed-step loop below, so connection events
      // and messages land on the same tick as the gameplay state they affect (spawning entities,
      // attaching PathFollowComponent), consistent with everything else Update() below does.
      for (ConnectionEvent &Event : Context->Network.PollConnectionEvents())
      {
        PendingConnectionEvents.push_back(Event);
      }
      for (NetworkMessage &Message : Context->Network.PollMessages())
      {
        PendingNetworkMessages.push_back(std::move(Message));
      }

      // UI layout/hit-testing happen here, before gameplay reads input, so this frame's pointer
      // claim (see InputManager::SetPointerClaimed) is already set by the time PreUpdate asks.
      Context->UICanvas.UpdateLayout(Renderer);
      Context->UICanvas.ProcessInput(Context->Input);

      // Once per real (rendered) frame, same cadence Canvas::ProcessInput above just ran at, and
      // before the fixed-step loop below so anything it latches this frame is visible to every
      // Update() call that follows, not delayed to the next real frame. This is where discrete
      // input (e.g. IsMouseButtonJustPressed) is safe to read: Update() below can run zero times
      // in a given real frame, so a one-frame input edge read there can be silently missed.
      if (EngineData)
      {
        EngineData->PreUpdate(*Context);
      }

      // Fixed update loop
      uint8_t UpdateCount{0};
      while (Lag >= FixedTimeStep && UpdateCount < MaxFrameSkip)
      {
        // Connection events fully before messages: both can arrive in the same Poll() above, and
        // a message can reference a connection whose spawn hasn't happened yet otherwise. Drains
        // fully on this loop's first iteration each real frame - later iterations see empty
        // deques and no-op, same as if this ran once per real frame instead of per tick.
        while (!PendingConnectionEvents.empty())
        {
          ConnectionEvent Event = PendingConnectionEvents.front();
          PendingConnectionEvents.pop_front();
          if (Event.Connected)
          {
            Context->Network.OnClientConnected().Broadcast(Event.ConnectionId);
          }
          else
          {
            Context->Network.OnClientDisconnected().Broadcast(Event.ConnectionId);
          }
        }
        while (!PendingNetworkMessages.empty())
        {
          NetworkMessage Message = std::move(PendingNetworkMessages.front());
          PendingNetworkMessages.pop_front();
          Engine::RPC::Dispatch(*Context, Message.ConnectionId, Message.Payload);
        }

        Context->World.RunSystems(FixedTimeStep);
        if (EngineData)
        {
          EngineData->Update(*Context, FixedTimeStep);
        }
        Lag -= FixedTimeStep;
        ++UpdateCount;
      }

      // Runs once per real frame, after this frame's Update() calls have moved things.
      if (EngineData)
      {
        EngineData->PostUpdate(*Context, DeltaTime);
      }

      MainCamera.PreRender();

      // Render only once per frame
      Context->Overlay.BeginFrame();
      if (EngineData)
      {
        EngineData->Draw(*Context);
      }

      // UI (and the debug overlay below) render in real screen pixels, not the pixel-art zoom
      // scale/viewport the game world just rendered at. Reset(), not just ResetScale(): PreRender
      // also set a small viewport (ScreenWidth x ScreenHeight) for the game world, and leaving
      // that active would clip UI rendering to that same small region.
      MainCamera.Reset();
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
      Context->Dispatcher.RemoveEventListener(SDL_EVENT_QUIT, SDLQuitEventId);
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
