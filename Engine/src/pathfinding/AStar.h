#pragma once

#include "Core.h"
#include "NavGrid.h"
#include <vector>

namespace Engine
{
  // Grid-based A* from Start to Goal, returning world-space cell-center waypoints from just after
  // Start's cell through Goal's cell, or empty if unreachable (or already at Goal's cell). Start
  // and Goal's own cell walkability is ignored when seeding the search, only neighbor traversal
  // enforces it, so a goal entity whose own collider happens to sit on a blocking layer doesn't
  // make every path to it fail.
  ENGINE_API std::vector<Vector2D> FindPath(const NavGrid &Grid, Vector2D Start, Vector2D Goal, NavMovement Movement = NavMovement::EightDirectional);

  // Cheap reachability-only check (no cost tracking or path reconstruction): can Goal be reached
  // from Start at all. Meant for validating a proposed obstacle placement doesn't fully seal the
  // only remaining path before committing to it, e.g. against a scratch copy of a NavGrid with
  // RasterizeShape applied to the proposed obstacle first.
  ENGINE_API bool HasPath(const NavGrid &Grid, Vector2D Start, Vector2D Goal, NavMovement Movement = NavMovement::EightDirectional);
}
