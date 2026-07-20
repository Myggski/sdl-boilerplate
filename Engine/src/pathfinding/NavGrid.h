#pragma once

#include "Core.h"
#include "src/math/Vector2D.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace Engine
{
  enum class NavMovement
  {
    FourDirectional,
    EightDirectional
  };

  // Bounded, dense grid (flat vector<bool> indexed by CellX/CellY), unlike CollisionSystem's
  // unbounded unordered_map broad phase - this is a fixed-size arena, sparsity buys nothing here.
  struct ENGINE_API NavGrid
  {
  public:
    bool IsInBounds(int32_t CellX, int32_t CellY) const;

    // False if out of bounds too, so callers don't need a separate IsInBounds check first.
    bool IsWalkable(int32_t CellX, int32_t CellY) const;

    int32_t CellIndex(int32_t CellX, int32_t CellY) const;

    // std::floor-based, like CollisionSystem::ToCellCoord (a truncating cast buckets negative
    // coordinates inconsistently at cell borders), but relative to Origin.
    int32_t WorldToCellX(float WorldX) const;
    int32_t WorldToCellY(float WorldY) const;
    Vector2D CellToWorldCenter(int32_t CellX, int32_t CellY) const;

  public:
    Vector2D Origin{0.0f, 0.0f}; // World-space position of cell (0,0)'s min corner.
    float CellSize = 32.0f;
    int32_t Width = 0;
    int32_t Height = 0;
    std::vector<bool> Blocked; // Width*Height, row-major: Index = CellY * Width + CellX.
  };

  // Game-declared: which collider Layers block pathing. Not ColliderComponent::IsStatic - a
  // deployable-wall trap is dynamic but still needs to block.
  struct ENGINE_API PathBlockingLayers
  {
    uint32_t Mask = 0;
  };

  // Calls Callback(NeighborCellX, NeighborCellY, StepCost) for each walkable neighbor reachable
  // from (CellX, CellY) under Movement. A diagonal step is only offered when both flanking
  // orthogonal cells are walkable too, so a path can never cut through a wall corner. Shared by
  // AStar.cpp and FlowField.cpp; no ENGINE_API, not meant for game code.
  void ForEachNeighbor(const NavGrid &Grid, int32_t CellX, int32_t CellY, NavMovement Movement,
                        const std::function<void(int32_t NeighborX, int32_t NeighborY, float StepCost)> &Callback);
}
