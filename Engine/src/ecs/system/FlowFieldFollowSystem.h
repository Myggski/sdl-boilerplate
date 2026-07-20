#pragma once

#include "Core.h"

namespace Engine
{
  class EntityManager;

  // Samples the FlowFieldSet resource for each entity's GoalId and steers VelocityComponent
  // toward it every tick; never sets any "arrived" state, the game itself detects that. A no-op
  // (zeroed velocity) for any entity whose GoalId has no matching field in the published
  // FlowFieldSet, including when nothing has been published yet. Auto-registered by
  // EngineContext at SystemPriority::Pathfinding, runs before Movement so the velocity it writes
  // gets integrated the same tick.
  ENGINE_API void FlowFieldFollowSystem(EntityManager &World, float DeltaTime);
}
