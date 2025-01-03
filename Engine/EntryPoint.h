#pragma once

#include "Core.h"
#include "GameEngine.h"

int main()
{
  std::unique_ptr<Engine::GameEngineData> EngineData{Engine::CreateGameEngineData()};

  // Check if function pointers are initialized
  if (!EngineData)
  {
    // Handle error or exit gracefully
    std::cerr << "Error: Engine initialization functions not set." << std::endl;
    return 1;
  }

  Engine::GameEngine Engine{Engine::GameEngine(std::move(EngineData))};
  Engine.Run();

  return 0;
}