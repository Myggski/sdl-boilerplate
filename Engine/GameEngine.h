#pragma once

#include "Core.h"
#include "PrecompiledHeader.h"
#include "EngineContext.h"

namespace Engine
{
  struct ENGINE_API GameEngineData
  {
  public:
    GameEngineData(
        const std::function<bool(Engine::EngineContext &)> &Startup,
        std::function<void(Engine::EngineContext &)> PreUpdate,
        std::function<void(Engine::EngineContext &, float)> Update,
        std::function<void(Engine::EngineContext &, float)> PostUpdate,
        std::function<void(Engine::EngineContext &)> Draw,
        std::function<void(Engine::EngineContext &)> Shutdown)
        : Startup(Startup),
          PreUpdate(PreUpdate),
          Update(Update),
          PostUpdate(PostUpdate),
          Draw(Draw),
          Shutdown(Shutdown) {}

  public:
    const std::function<bool(Engine::EngineContext &)> Startup;
    const std::function<void(Engine::EngineContext &)> PreUpdate;
    const std::function<void(Engine::EngineContext &, float)> Update;
    const std::function<void(Engine::EngineContext &, float)> PostUpdate;
    const std::function<void(Engine::EngineContext &)> Draw;
    const std::function<void(Engine::EngineContext &)> Shutdown;
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
    std::unique_ptr<EngineContext> Context;

    SDL_Window *Window{nullptr};
    SDL_Renderer *Renderer{nullptr};

    bool IsGameRunning{false};
    int SDLQuitEventId{-1};
  };

  extern std::unique_ptr<GameEngineData> CreateGameEngineData();
}
