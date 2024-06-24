#pragma once

#include "Core.h"
#include "PrecompiledHeader.h"

struct SDL_Window;

namespace Engine
{
  struct ENGINE_API GameEngineData
  {
  public:
    GameEngineData(
        const std::function<bool()> &Initialize,
        std::function<void(float)> Update,
        std::function<void()> Shutdown)
        : Initialize(Initialize),
          Update(Update),
          Shutdown(Shutdown) {}

  public:
    const std::function<bool()> Initialize;
    const std::function<void(float)> Update;
    const std::function<void()> Shutdown;
  };

  class ENGINE_API GameEngine
  {
  public:
    GameEngine(GameEngineData *EngineData)
        : EngineData(std::move(EngineData)) {};

    void Run();

  private:
    bool Initialize();
    void Update();
    void Shutdown();

  private:
    GameEngineData *EngineData;
    SDL_Window *Window;

    bool IsGameRunning;
  };

  extern GameEngineData *CreateGameEngineData();
}