#include "NavGridBuilder.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/TransformComponent.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace Engine
{
  namespace
  {
    // WorldToCellX/Y (a plain floor) is correct and unambiguous for mapping a single point to a
    // cell. It's the wrong tool for the LAST cell an interval's upper bound touches though: each
    // cell owns a half-open [CellStart, CellStart + CellSize) range, so an upper bound landing
    // exactly on a cell boundary should stay in the cell below that boundary, not bleed into the
    // next one. floor(UpperBound / CellSize) does exactly that bleed (floor(144/16) == 9, not the
    // 8 a shape actually ending at world x=144 should occupy); ceil-minus-one doesn't.
    int32_t WorldToMaxCellCoord(float UpperBound, float Origin, float CellSize)
    {
      return static_cast<int32_t>(std::ceil((UpperBound - Origin) / CellSize)) - 1;
    }
  }

  void RasterizeShape(NavGrid &Grid, Vector2D Center, ColliderShape Shape, Vector2D HalfExtentsOrRadius)
  {
    float Radius = HalfExtentsOrRadius.X;
    float HalfWidth = Shape == ColliderShape::AABB ? HalfExtentsOrRadius.X : Radius;
    float HalfHeight = Shape == ColliderShape::AABB ? HalfExtentsOrRadius.Y : Radius;

    int32_t MinCellX = std::max(0, Grid.WorldToCellX(Center.X - HalfWidth));
    int32_t MaxCellX = std::min(Grid.Width - 1, WorldToMaxCellCoord(Center.X + HalfWidth, Grid.Origin.X, Grid.CellSize));
    int32_t MinCellY = std::max(0, Grid.WorldToCellY(Center.Y - HalfHeight));
    int32_t MaxCellY = std::min(Grid.Height - 1, WorldToMaxCellCoord(Center.Y + HalfHeight, Grid.Origin.Y, Grid.CellSize));

    for (int32_t CellY = MinCellY; CellY <= MaxCellY; ++CellY)
    {
      for (int32_t CellX = MinCellX; CellX <= MaxCellX; ++CellX)
      {
        if (Shape == ColliderShape::AABB)
        {
          Grid.Blocked[static_cast<size_t>(Grid.CellIndex(CellX, CellY))] = true;
          continue;
        }

        // Circle: clamp the circle's center to this cell's own box extents (the closest point on
        // the cell to that center), the same technique CollisionSystem's ShapesOverlap uses for
        // its AABB-vs-Circle branch, so corner cells outside the actual circle aren't falsely
        // blocked.
        Vector2D CellCenter = Grid.CellToWorldCenter(CellX, CellY);
        float CellHalf = Grid.CellSize * 0.5f;

        float ClampedX = std::max(CellCenter.X - CellHalf, std::min(Center.X, CellCenter.X + CellHalf));
        float ClampedY = std::max(CellCenter.Y - CellHalf, std::min(Center.Y, CellCenter.Y + CellHalf));
        Vector2D Clamped{ClampedX, ClampedY};

        if (Clamped.DistanceSquared(Center) <= Radius * Radius)
        {
          Grid.Blocked[static_cast<size_t>(Grid.CellIndex(CellX, CellY))] = true;
        }
      }
    }
  }

  void RebuildNavGrid(EntityManager &World, Vector2D Origin, int32_t WidthCells, int32_t HeightCells, float CellSize)
  {
    NavGrid Grid;
    Grid.Origin = Origin;
    Grid.CellSize = CellSize;
    Grid.Width = WidthCells;
    Grid.Height = HeightCells;
    Grid.Blocked.assign(static_cast<size_t>(WidthCells) * static_cast<size_t>(HeightCells), false);

    PathBlockingLayers *Layers = World.GetResource<PathBlockingLayers>();
    uint32_t Mask = Layers ? Layers->Mask : 0;

    if (Mask != 0)
    {
      World.ForEach<TransformComponent, ColliderComponent>(
          [&Grid, Mask](Entity, TransformComponent &Transform, ColliderComponent &Collider)
          {
            if ((Collider.Layer & Mask) == 0)
            {
              return;
            }

            Vector2D Center = Transform.Position + Collider.Offset;
            Vector2D HalfExtentsOrRadius = Collider.Shape == ColliderShape::AABB
                                                ? Collider.HalfExtents
                                                : Vector2D{Collider.Radius, Collider.Radius};

            RasterizeShape(Grid, Center, Collider.Shape, HalfExtentsOrRadius);
          });
    }

    World.SetResource<NavGrid>(std::move(Grid));
  }
}
