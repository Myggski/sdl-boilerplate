#pragma once

#include "Core.h"

struct SDL_Window;
struct SDL_Renderer;

namespace Engine
{
  class SDLEventDispatcher;
  class Camera;

  // Owns Dear ImGui's setup/teardown and per-frame NewFrame/Render bracketing, shared by any
  // Game (or future Editor) built on the engine so neither has to touch Dear ImGui setup itself.
  // Only meaningful when ENGINE_WITH_DEBUG_UI is defined (Debug builds); Dear ImGui's own source
  // is never compiled into a Release build at all, so every method here is a no-op in Release,
  // not just unused. This class is still always declared and always an EngineContext member so
  // consuming code never needs to #ifdef around its existence, only around code that calls into
  // the ImGui:: API directly (which isn't available to link against in Release).
  class ENGINE_API DebugOverlay
  {
  public:
    DebugOverlay(SDL_Window *Window, SDL_Renderer *Renderer, SDLEventDispatcher &Dispatcher);
    ~DebugOverlay();

    DebugOverlay(const DebugOverlay &) = delete;
    DebugOverlay &operator=(const DebugOverlay &) = delete;

    void BeginFrame();
    void EndFrame(SDL_Renderer *Renderer, Camera &MainCamera);

  private:
    SDLEventDispatcher &Dispatcher;
    int32_t SDLEventHandle{-1};
  };
}
