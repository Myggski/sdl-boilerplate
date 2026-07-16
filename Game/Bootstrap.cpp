#include "Game.h"
#include "GameEngine.h"
#include <memory>

// Wires the engine to the Game::Startup/Update/Draw/Shutdown callbacks. Framework glue, not
// game-specific; there's no need to touch this file to make a game.
namespace Engine
{
  std::unique_ptr<GameEngineData> CreateGameEngineData()
  {
    return std::make_unique<GameEngineData>(
        Game::Startup,
        Game::Update,
        Game::Draw,
        Game::Shutdown);
  }
}
