#pragma once

#include "Core.h"
#include "src/math/Vector2D.h"

namespace Engine
{
  enum class ColliderShape
  {
    AABB,
    Circle
  };

  // One component covers both shapes (the ECS's ForEach is AND-only, so one shape-dispatching
  // type is far simpler than two types needing OR-style queries to let AABB and Circle collide
  // with each other). CollisionSystem reads Shape to decide which fields (HalfExtents vs Radius)
  // apply. Offset/HalfExtents/Radius are deliberately center-based (Offset is added to
  // TransformComponent::Position to get this collider's CENTER), unlike Rect's top-left
  // convention used for rendering: circle/AABB overlap math wants a center.
  //
  // No Mask: which layers interact is declared once, symmetrically, in a CollisionMatrix (see
  // CollisionMatrix.h) rather than per-collider, so there is one place to edit a relationship
  // instead of two masks that can drift out of sync.
  struct ENGINE_API ColliderComponent
  {
  public:
    ColliderComponent();
    // AABB: Vector2D disambiguates from the Circle constructor below (no implicit float->Vector2D
    // conversion exists), so no named-factory indirection is needed.
    ColliderComponent(Vector2D HalfExtents, uint32_t Layer, bool IsStatic = false, Vector2D Offset = {0.0f, 0.0f});
    // Circle
    ColliderComponent(float Radius, uint32_t Layer, bool IsStatic = false, Vector2D Offset = {0.0f, 0.0f});

  public:
    ColliderShape Shape = ColliderShape::AABB;
    Vector2D Offset{0.0f, 0.0f};
    Vector2D HalfExtents{8.0f, 8.0f}; // AABB only
    float Radius = 8.0f;              // Circle only
    uint32_t Layer = 1;
    bool IsStatic = false;
  };
}
