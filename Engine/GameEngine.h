#pragma once

#include "Core.h"
#include "PrecompiledHeader.h"
#include "sdl/SDLEventDispatcher.h"

struct SDL_Window;
struct SDL_Renderer;

namespace Engine
{
  class Application;

  struct ENGINE_API GameEngineData
  {
  public:
    GameEngineData(
        const std::function<bool(SDL_Window *, SDL_Renderer *, Engine::SDLEventDispatcher *)> &Initialize,
        std::function<void(float)> Update,
        std::function<void(SDL_Renderer *)> Draw,
        std::function<void()> Shutdown)
        : Initialize(Initialize),
          Update(Update),
          Draw(Draw),
          Shutdown(Shutdown) {}

  public:
    const std::function<bool(SDL_Window *, SDL_Renderer *, Engine::SDLEventDispatcher *)> Initialize;
    const std::function<void(float)> Update;
    const std::function<void(SDL_Renderer *)> Draw;
    const std::function<void()> Shutdown;
  };

  class ENGINE_API GameEngine
  {
  public:
    GameEngine(std::unique_ptr<GameEngineData> EngineData)
        : EngineData(std::move(EngineData)) {};

    void Run();

  private:
    bool Initialize();
    void Update();
    void Shutdown();
    void Cleanup();

  private:
    std::unique_ptr<GameEngineData> EngineData{nullptr};
    Engine::SDLEventDispatcher Dispatcher{};

    SDL_Window *Window{nullptr};
    SDL_Renderer *Renderer{nullptr};

    bool IsGameRunning{false};
    int SDLQuitEventId{-1};
  };

  extern std::unique_ptr<GameEngineData> CreateGameEngineData();
}