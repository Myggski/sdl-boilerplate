#include "DebugOverlay.h"

#ifdef ENGINE_WITH_DEBUG_UI
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "sdl/SDLEventDispatcher.h"
#include "Camera.h"
#include <SDL3/SDL.h>
#endif

namespace Engine
{
#ifdef ENGINE_WITH_DEBUG_UI

  DebugOverlay::DebugOverlay(SDL_Window *Window, SDL_Renderer *Renderer, SDLEventDispatcher &Dispatcher)
      : Dispatcher(Dispatcher)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    SDLEventHandle = Dispatcher.GetSDLEvent().Add([](const SDL_Event &Event)
                                                   { ImGui_ImplSDL3_ProcessEvent(&Event); });

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(Window, Renderer);
    ImGui_ImplSDLRenderer3_Init(Renderer);
  }

  DebugOverlay::~DebugOverlay()
  {
    Dispatcher.GetSDLEvent().Remove(SDLEventHandle);

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }

  void DebugOverlay::BeginFrame()
  {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
  }

  void DebugOverlay::EndFrame(SDL_Renderer *Renderer, Camera &MainCamera)
  {
    ImGui::Render();

    // ImGui draws in screen pixels; Reset() (not ResetScale()) also clears the game-world
    // viewport, which would otherwise clip ImGui's draw calls.
    MainCamera.Reset();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), Renderer);
    MainCamera.SetZoomScale();
  }

  ImGuiContext *DebugOverlay::GetImGuiContext() const
  {
    return ImGui::GetCurrentContext();
  }

#else

  DebugOverlay::DebugOverlay(SDL_Window *, SDL_Renderer *, SDLEventDispatcher &Dispatcher)
      : Dispatcher(Dispatcher) {}
  DebugOverlay::~DebugOverlay() = default;
  void DebugOverlay::BeginFrame() {}
  void DebugOverlay::EndFrame(SDL_Renderer *, Camera &) {}
  ImGuiContext *DebugOverlay::GetImGuiContext() const { return nullptr; }

#endif
}
