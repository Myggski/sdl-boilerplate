#include "NavGrid.h"
#include <cmath>

namespace Engine
{
  bool NavGrid::IsInBounds(int32_t CellX, int32_t CellY) const
  {
    return CellX >= 0 && CellX < Width && CellY >= 0 && CellY < Height;
  }

  bool NavGrid::IsWalkable(int32_t CellX, int32_t CellY) const
  {
    return IsInBounds(CellX, CellY) && !Blocked[static_cast<size_t>(CellIndex(CellX, CellY))];
  }

  int32_t NavGrid::CellIndex(int32_t CellX, int32_t CellY) const
  {
    return CellY * Width + CellX;
  }

  int32_t NavGrid::WorldToCellX(float WorldX) const
  {
    return static_cast<int32_t>(std::floor((WorldX - Origin.X) / CellSize));
  }

  int32_t NavGrid::WorldToCellY(float WorldY) const
  {
    return static_cast<int32_t>(std::floor((WorldY - Origin.Y) / CellSize));
  }

  Vector2D NavGrid::CellToWorldCenter(int32_t CellX, int32_t CellY) const
  {
    return Vector2D{
        Origin.X + (static_cast<float>(CellX) + 0.5f) * CellSize,
        Origin.Y + (static_cast<float>(CellY) + 0.5f) * CellSize};
  }

  namespace
  {
    constexpr float DiagonalCost = 1.41421356f; // sqrt(2)

    struct NeighborOffset
    {
      int32_t DeltaX;
      int32_t DeltaY;
      float Cost;
      bool IsDiagonal;
    };

    // Orthogonal offsets first: FourDirectional mode only takes the first 4 of these.
    constexpr NeighborOffset Offsets[8] = {
        {1, 0, 1.0f, false},
        {-1, 0, 1.0f, false},
        {0, 1, 1.0f, false},
        {0, -1, 1.0f, false},
        {1, 1, DiagonalCost, true},
        {1, -1, DiagonalCost, true},
        {-1, 1, DiagonalCost, true},
        {-1, -1, DiagonalCost, true},
    };
  }

  void ForEachNeighbor(const NavGrid &Grid, int32_t CellX, int32_t CellY, NavMovement Movement,
                        const std::function<void(int32_t, int32_t, float)> &Callback)
  {
    size_t OffsetCount = Movement == NavMovement::EightDirectional ? 8 : 4;
    for (size_t Index = 0; Index < OffsetCount; ++Index)
    {
      const NeighborOffset &Offset = Offsets[Index];
      int32_t NeighborX = CellX + Offset.DeltaX;
      int32_t NeighborY = CellY + Offset.DeltaY;

      if (!Grid.IsWalkable(NeighborX, NeighborY))
      {
        continue;
      }

      if (Offset.IsDiagonal)
      {
        // Both flanking orthogonal cells must be walkable too, otherwise this diagonal step
        // would cut through a wall corner.
        if (!Grid.IsWalkable(CellX + Offset.DeltaX, CellY) || !Grid.IsWalkable(CellX, CellY + Offset.DeltaY))
        {
          continue;
        }
      }

      Callback(NeighborX, NeighborY, Offset.Cost);
    }
  }
}
