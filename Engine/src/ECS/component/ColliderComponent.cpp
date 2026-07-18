#include "ColliderComponent.h"

namespace Engine
{
  ColliderComponent::ColliderComponent() = default;

  ColliderComponent::ColliderComponent(Vector2D HalfExtents, uint32_t Layer, bool IsStatic, Vector2D Offset)
      : Shape(ColliderShape::AABB), Offset(Offset), HalfExtents(HalfExtents), Layer(Layer), IsStatic(IsStatic) {}

  ColliderComponent::ColliderComponent(float Radius, uint32_t Layer, bool IsStatic, Vector2D Offset)
      : Shape(ColliderShape::Circle), Offset(Offset), Radius(Radius), Layer(Layer), IsStatic(IsStatic) {}
}
