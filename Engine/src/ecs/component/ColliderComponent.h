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

  // One component covers both shapes (the ECS's ForEach is AND-only, so a single
  // shape-dispatching type is simpler than two types needing OR-style queries). Offset is added
  // to TransformComponent::Position to get this collider's CENTER, unlike Rect's top-left
  // convention - overlap math wants a center.
  //
  // No Mask: layer interactions are declared once, symmetrically, in CollisionMatrix instead.
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
