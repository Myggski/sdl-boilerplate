#include "CollisionSystem.h"
#include "src/ecs/CollisionMatrix.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/ColliderComponent.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace Engine
{
  namespace
  {
    struct ColliderEntry
    {
      Entity Owner;
      Vector2D Center;
      ColliderShape Shape;
      Vector2D HalfExtents;
      float Radius;
      uint32_t Layer;
      bool IsStatic;
    };

    struct GridCell
    {
      int32_t CellX;
      int32_t CellY;

      bool operator==(const GridCell &Other) const
      {
        return CellX == Other.CellX && CellY == Other.CellY;
      }
    };

    struct GridCellHash
    {
      size_t operator()(const GridCell &Cell) const
      {
        size_t HashX = std::hash<int32_t>{}(Cell.CellX);
        size_t HashY = std::hash<int32_t>{}(Cell.CellY);
        return HashX ^ (HashY * 2654435761u + 0x9e3779b9u + (HashX << 6) + (HashX >> 2));
      }
    };

    // std::floor, not a truncating cast: a plain (int32_t)(Value / CellSize) truncates toward
    // zero, which buckets negative coordinates inconsistently at cell borders (e.g. -0.5 and 0.5
    // would both truncate to cell 0 instead of -1/0) and can silently drop pairs near them.
    int32_t ToCellCoord(float Value, float CellSize)
    {
      return static_cast<int32_t>(std::floor(Value / CellSize));
    }

    bool ShapesOverlap(const ColliderEntry &A, const ColliderEntry &B)
    {
      if (A.Shape == ColliderShape::AABB && B.Shape == ColliderShape::AABB)
      {
        return std::abs(A.Center.X - B.Center.X) <= (A.HalfExtents.X + B.HalfExtents.X) &&
               std::abs(A.Center.Y - B.Center.Y) <= (A.HalfExtents.Y + B.HalfExtents.Y);
      }

      if (A.Shape == ColliderShape::Circle && B.Shape == ColliderShape::Circle)
      {
        float RadiusSum = A.Radius + B.Radius;
        return A.Center.DistanceSquared(B.Center) <= RadiusSum * RadiusSum;
      }

      // One AABB, one Circle: clamp the circle's center to the AABB's extents (the closest point
      // on the box to that center), then compare that clamped point's distance to the circle's
      // center against its radius.
      const ColliderEntry &Box = A.Shape == ColliderShape::AABB ? A : B;
      const ColliderEntry &Ball = A.Shape == ColliderShape::AABB ? B : A;

      float ClampedX = std::max(Box.Center.X - Box.HalfExtents.X, std::min(Ball.Center.X, Box.Center.X + Box.HalfExtents.X));
      float ClampedY = std::max(Box.Center.Y - Box.HalfExtents.Y, std::min(Ball.Center.Y, Box.Center.Y + Box.HalfExtents.Y));
      Vector2D Clamped{ClampedX, ClampedY};

      return Clamped.DistanceSquared(Ball.Center) <= Ball.Radius * Ball.Radius;
    }
  }

  void CollisionSystem(EntityManager &World, float DeltaTime, float CellSize)
  {
    // No matrix published means nothing is configured to collide with anything; still publish an
    // empty result rather than leaving a stale one from a previous tick in place.
    CollisionMatrix *Matrix = World.GetResource<CollisionMatrix>();
    if (!Matrix)
    {
      World.SetResource<CollisionResults>(CollisionResults{});
      return;
    }

    // Pass 1: resolve every Transform+Collider entity once into a flat array, avoiding repeated
    // GetComponent calls in the passes below.
    std::vector<ColliderEntry> Entries;
    World.ForEach<TransformComponent, ColliderComponent>(
        [&Entries](Entity Owner, TransformComponent &Transform, ColliderComponent &Collider)
        {
          Entries.push_back(ColliderEntry{
              Owner,
              Transform.Position + Collider.Offset,
              Collider.Shape,
              Collider.HalfExtents,
              Collider.Radius,
              Collider.Layer,
              Collider.IsStatic});
        });

    // Pass 2 (broad phase): insert each entry's index into every grid cell its bounding box
    // spans, not just its center cell, avoiding missing pairs at cell borders without needing a
    // separate neighbor-widening step.
    std::unordered_map<GridCell, std::vector<size_t>, GridCellHash> Grid;
    for (size_t Index = 0; Index < Entries.size(); ++Index)
    {
      const ColliderEntry &Entry = Entries[Index];
      float HalfWidth = Entry.Shape == ColliderShape::AABB ? Entry.HalfExtents.X : Entry.Radius;
      float HalfHeight = Entry.Shape == ColliderShape::AABB ? Entry.HalfExtents.Y : Entry.Radius;

      int32_t MinCellX = ToCellCoord(Entry.Center.X - HalfWidth, CellSize);
      int32_t MaxCellX = ToCellCoord(Entry.Center.X + HalfWidth, CellSize);
      int32_t MinCellY = ToCellCoord(Entry.Center.Y - HalfHeight, CellSize);
      int32_t MaxCellY = ToCellCoord(Entry.Center.Y + HalfHeight, CellSize);

      for (int32_t CellY = MinCellY; CellY <= MaxCellY; ++CellY)
      {
        for (int32_t CellX = MinCellX; CellX <= MaxCellX; ++CellX)
        {
          Grid[GridCell{CellX, CellY}].push_back(Index);
        }
      }
    }

    // Pass 3 (dedup + filter + narrow phase): an entity can share multiple cells with another, so
    // dedup by index pair FIRST, before any other check, otherwise a pair sharing several cells
    // would redundantly re-run the filter/narrow-phase work once per shared cell.
    std::unordered_set<uint64_t> TestedPairs;
    std::vector<CollisionPair> Pairs;

    for (auto &[Cell, Indices] : Grid)
    {
      for (size_t First = 0; First < Indices.size(); ++First)
      {
        for (size_t Second = First + 1; Second < Indices.size(); ++Second)
        {
          size_t IndexA = std::min(Indices[First], Indices[Second]);
          size_t IndexB = std::max(Indices[First], Indices[Second]);

          uint64_t TestKey = (static_cast<uint64_t>(IndexA) << 32) | static_cast<uint64_t>(IndexB);
          if (!TestedPairs.insert(TestKey).second)
          {
            continue;
          }

          const ColliderEntry &EntryA = Entries[IndexA];
          const ColliderEntry &EntryB = Entries[IndexB];

          // Two static colliders never need reporting, that's world geometry touching itself,
          // not an event.
          if (EntryA.IsStatic && EntryB.IsStatic)
          {
            continue;
          }

          if (!Matrix->DoLayersCollide(EntryA.Layer, EntryB.Layer))
          {
            continue;
          }

          if (ShapesOverlap(EntryA, EntryB))
          {
            Pairs.push_back(CollisionPair{EntryA.Owner, EntryB.Owner});
          }
        }
      }
    }

    World.SetResource<CollisionResults>(CollisionResults{std::move(Pairs)});
  }
}
