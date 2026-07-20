#pragma once

#include "Core.h"

namespace Engine
{
  class EntityManager;

  // Integrates VelocityComponent into TransformComponent::Position every tick. Basic enough that
  // most 2D games want it as-is; opt-in like any other system, register it yourself, e.g.
  //   Context.World.RegisterSystem([&Context](float DeltaTime)
  //     { MovementSystem(Context.World, DeltaTime); });
  ENGINE_API void MovementSystem(EntityManager &World, float DeltaTime);
}
