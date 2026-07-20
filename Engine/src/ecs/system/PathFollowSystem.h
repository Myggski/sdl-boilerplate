#pragma once

#include "Core.h"

namespace Engine
{
  class EntityManager;

  // Steers every entity with a Transform + Velocity + PathFollowComponent toward its current
  // waypoint, advancing through the list as each is reached; sets HasArrived and zeroes velocity
  // once the list is exhausted. Auto-registered by EngineContext at SystemPriority::Pathfinding,
  // runs before Movement so the velocity it writes gets integrated the same tick.
  ENGINE_API void PathFollowSystem(EntityManager &World, float DeltaTime);
}
