#include "EngineContext.h"
#include "ecs/SystemPriority.h"
#include "ecs/system/MovementSystem.h"
#include "ecs/system/AnimationSystem.h"
#include "ecs/system/CollisionSystem.h"

namespace Engine
{
  EngineContext::EngineContext(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t Zoom)
      : Window(Window), Renderer(Renderer), Dispatcher(),
        Assets(Renderer), Input(Dispatcher), MainCamera(Window, Renderer, ScreenWidth, ScreenHeight, Zoom),
        Overlay(Window, Renderer, Dispatcher)
  {
    // Every game needs these, so they're registered here rather than left for each game to
    // remember to add itself. Priorities from Engine::SystemPriority; a game inserting its own
    // system should pick a priority relative to those constants, not a guessed number.
    World.RegisterSystem([this](float DeltaTime)
                         { MovementSystem(World, DeltaTime); },
                         SystemPriority::Movement);
    World.RegisterSystem([this](float DeltaTime)
                         { AnimationSystem(World, DeltaTime); },
                         SystemPriority::Animation);
    World.RegisterSystem([this](float DeltaTime)
                         { CollisionSystem(World, DeltaTime); },
                         SystemPriority::Collision);
  }
}
