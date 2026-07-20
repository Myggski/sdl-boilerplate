#pragma once

#include "Core.h"
#include "NavGrid.h"
#include "src/ecs/component/ColliderComponent.h"

namespace Engine
{
  class EntityManager;

  // Marks every cell Shape's footprint (centered at Center) overlaps as blocked in Grid. Does not
  // clear previously-blocked cells, callers rasterizing multiple obstacles into the same Grid
  // should call this once per obstacle to union their footprints. HalfExtentsOrRadius.X is used
  // as Radius when Shape is Circle (Y ignored); both X and Y are used as HalfExtents when Shape
  // is AABB.
  ENGINE_API void RasterizeShape(NavGrid &Grid, Vector2D Center, ColliderShape Shape, Vector2D HalfExtentsOrRadius);

  // Rebuilds and publishes a NavGrid resource from scratch: every entity with a Transform +
  // Collider whose Collider.Layer is set in World.GetResource<PathBlockingLayers>()'s Mask gets
  // rasterized as blocked (no resource published means an all-open grid, mirroring
  // CollisionSystem's "no matrix means nothing collides" default-safe behavior). Called explicitly
  // by game code whenever the obstacle layout changes (a trap placed/removed/moved); not
  // auto-registered by EngineContext like Movement/Animation/Collision, rebuilding every tick
  // would be pure overhead most frames since layout rarely changes frame to frame.
  ENGINE_API void RebuildNavGrid(EntityManager &World, Vector2D Origin, int32_t WidthCells, int32_t HeightCells, float CellSize);
}
