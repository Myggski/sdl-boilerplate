#pragma once

#include "Core.h"

struct SDL_Window;
struct SDL_Renderer;
struct ImGuiContext;

namespace Engine
{
  class SDLEventDispatcher;
  class Camera;

  // Owns Dear ImGui's setup/teardown and per-frame NewFrame/Render bracketing. No-op in Release
  // (ENGINE_WITH_DEBUG_UI undefined); always declared so consuming code doesn't need to #ifdef
  // around its existence, only around direct ImGui:: calls.
  class ENGINE_API DebugOverlay
  {
  public:
    DebugOverlay(SDL_Window *Window, SDL_Renderer *Renderer, SDLEventDispatcher &Dispatcher);
    ~DebugOverlay();

    DebugOverlay(const DebugOverlay &) = delete;
    DebugOverlay &operator=(const DebugOverlay &) = delete;

    void BeginFrame();
    void EndFrame(SDL_Renderer *Renderer, Camera &MainCamera);

    // Engine.dll and Game.exe each link their own separate copy of ImGui (static lib), so their
    // global contexts start out independent. Pass this to ImGui::SetCurrentContext() in any other
    // linked copy to point it at this one (nullptr in Release).
    ImGuiContext *GetImGuiContext() const;

  private:
    SDLEventDispatcher &Dispatcher;
    int32_t SDLEventHandle{-1};
  };
}
