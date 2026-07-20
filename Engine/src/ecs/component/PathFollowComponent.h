#pragma once

#include "Core.h"
#include "src/math/Vector2D.h"
#include <cstddef>
#include <vector>

namespace Engine
{
  // Steers VelocityComponent toward each waypoint in turn (see PathFollowSystem), typically
  // populated from AStar::FindPath. HasArrived is polled by game code once set (e.g.
  // GetComponent<PathFollowComponent>(Entity)->HasArrived), matching CollisionResults's existing
  // "publish once, poll every tick" idiom rather than a callback.
  struct ENGINE_API PathFollowComponent
  {
  public:
    std::vector<Vector2D> Waypoints; // World-space cell centers.
    size_t CurrentIndex = 0;
    float Speed = 60.0f;        // World units/second.
    float ArrivalRadius = 4.0f; // Distance to the current waypoint counted as "reached".
    bool HasArrived = false;
  };
}
