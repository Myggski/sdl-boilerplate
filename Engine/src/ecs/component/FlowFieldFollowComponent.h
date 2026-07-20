#pragma once

#include "Core.h"
#include "src/pathfinding/FlowField.h"

namespace Engine
{
  // Samples a published FlowFieldSet's field for GoalId each tick and steers VelocityComponent
  // toward it (see FlowFieldFollowSystem). No arrival state: the game itself detects "reached the
  // goal" (e.g. a proximity/collision check against the base), unlike PathFollowComponent this
  // never completes on its own.
  struct ENGINE_API FlowFieldFollowComponent
  {
  public:
    FlowFieldId GoalId = 0;
    float Speed = 60.0f;
  };
}
