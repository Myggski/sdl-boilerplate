#pragma once

#include "Core.h"
#include "NavGrid.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Engine
{
  using FlowFieldId = uint32_t; // Game-chosen key, e.g. a base entity's Index().

  // Covers many-agents-one-goal pathing (wave enemies converging on a base): built once per goal,
  // every agent then just samples its current cell's direction each tick instead of running its
  // own search. Carries its own copy of the grid dimensions/origin/cell size it was built
  // against, not just an implicit reference to "whatever NavGrid is currently published", so
  // rebuilding NavGrid with different bounds later can't silently desync an already-built field's
  // cell indexing.
  struct ENGINE_API FlowField
  {
  public:
    // Mirrors NavGrid's own coordinate helpers (see NavGrid.h), kept on FlowField itself rather
    // than routing through whatever NavGrid happens to be currently published, since this field's
    // Origin/CellSize/dimensions are its own frozen copy, not necessarily still in sync with it.
    bool IsInBounds(int32_t CellX, int32_t CellY) const;
    int32_t CellIndex(int32_t CellX, int32_t CellY) const;
    int32_t WorldToCellX(float WorldX) const;
    int32_t WorldToCellY(float WorldY) const;

  public:
    Vector2D Origin{0.0f, 0.0f};
    float CellSize = 32.0f;
    int32_t Width = 0;
    int32_t Height = 0;
    std::vector<float> Cost;          // Integration field; unreachable/blocked stays infinity.
    std::vector<Vector2D> Directions; // Per-cell unit steering direction; {0,0} at goal/unreachable.
  };

  // SetResource<T> only holds one instance per type; this wraps a map since more than one goal
  // could plausibly be active at once even though only one is used at launch.
  struct ENGINE_API FlowFieldSet
  {
  public:
    std::unordered_map<FlowFieldId, FlowField> Fields;
  };

  // Two-pass build: an integration pass (Dijkstra seeded at Goal's cell, cost 0) then a flow pass
  // (every walkable cell points toward whichever neighbor has the lowest finite cost). Goal's own
  // cell walkability is ignored when seeding, same as AStar::FindPath, so a goal entity's own
  // collider footprint doesn't make it unreachable from itself.
  ENGINE_API FlowField BuildFlowField(const NavGrid &Grid, Vector2D Goal, NavMovement Movement = NavMovement::EightDirectional);
}
