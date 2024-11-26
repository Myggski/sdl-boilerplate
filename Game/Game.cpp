#include "Game.h"
#include "Engine.h"
#include <SDL.h>
#include "imgui.h"
#include <memory>
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

namespace Game
{
  // Definition of static member variable
  bool Initialize(SDL_Window *Window, SDL_Renderer *Renderer)
  {
    // Perform drawing
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    Engine::EventManager::GetInstance().GetSDLEvent().Add([](const SDL_Event &Event)
                                                          { ImGui_ImplSDL2_ProcessEvent(&Event); });
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(Window, Renderer);
    ImGui_ImplSDLRenderer2_Init(Renderer);

    // Perform initialization logic here
    return true; // Example return value
  }

  // Definition of static update function
  void Update(float DeltaTime)
  {
    std::cout << "is D pressed: " << Engine::InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_D) << " hold: " << Engine::InputManager::GetInstance().IsKeyHeld(SDL_SCANCODE_D) << "\n";
  }

  void Draw(SDL_Renderer *Renderer)
  {
    // Perform update logic here
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    bool Show{true};
    ImGui::ShowDemoWindow(&Show);

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255); // Clear color
    SDL_RenderClear(Renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), Renderer);
    SDL_RenderPresent(Renderer);
  }

  // Definition of static shutdown function
  void Shutdown()
  {
    // Perform shutdown logic here
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }
}

namespace Engine
{
  std::unique_ptr<GameEngineData> CreateGameEngineData()
  {
    return std::make_unique<GameEngineData>(
        Game::Initialize,
        Game::Update,
        Game::Draw,
        Game::Shutdown);
  }
}