#include "Game.h"
#include "Engine.h"

namespace Game
{
  // Definition of static member variable
  bool Initialize()
  {
    // Perform initialization logic here
    return true; // Example return value
  }

  // Definition of static update function
  void Update(float DeltaTime)
  {
    // Perform update logic here
  }

  // Definition of static shutdown function
  void Shutdown()
  {
    // Perform shutdown logic here
  }
}

namespace Engine
{
  GameEngineData *CreateGameEngineData()
  {
    GameEvent<int> Event;
    Event.Add([](int value)
              { std::cout << value << "\n"; });

    Event.Broadcast(10);

    return new GameEngineData(
        Game::Initialize,
        Game::Update,
        Game::Shutdown);
  }
}