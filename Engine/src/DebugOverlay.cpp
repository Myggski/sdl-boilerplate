#include "DebugOverlay.h"

#ifdef ENGINE_WITH_DEBUG_UI
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "sdl/SDLEventDispatcher.h"
#include "Camera.h"
#include <SDL.h>
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
                                                   { ImGui_ImplSDL2_ProcessEvent(&Event); });

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(Window, Renderer);
    ImGui_ImplSDLRenderer2_Init(Renderer);
  }

  DebugOverlay::~DebugOverlay()
  {
    Dispatcher.GetSDLEvent().Remove(SDLEventHandle);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  void DebugOverlay::BeginFrame()
  {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
  }

  void DebugOverlay::EndFrame(SDL_Renderer *Renderer, Camera &MainCamera)
  {
    ImGui::Render();

    // ImGui draws in screen pixels; the camera's pixel-art zoom scale must not apply to it.
    MainCamera.ResetScale();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), Renderer);
    MainCamera.SetZoomScale();
  }

#else

  DebugOverlay::DebugOverlay(SDL_Window *, SDL_Renderer *, SDLEventDispatcher &Dispatcher)
      : Dispatcher(Dispatcher) {}
  DebugOverlay::~DebugOverlay() = default;
  void DebugOverlay::BeginFrame() {}
  void DebugOverlay::EndFrame(SDL_Renderer *, Camera &) {}

#endif
}
