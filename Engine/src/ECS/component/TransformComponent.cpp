#include "TransformComponent.h"

namespace Engine
{
  TransformComponent::TransformComponent() = default;
  TransformComponent::TransformComponent(Vector2D Position, float Rotation, Vector2D Scale)
      : Position(Position), Scale(Scale), Rotation(Rotation) {}
}
