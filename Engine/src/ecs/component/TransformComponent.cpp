#include "TransformComponent.h"
#include "src/ecs/ComponentReplication.h"

namespace Engine
{
  TransformComponent::TransformComponent() = default;
  TransformComponent::TransformComponent(Vector2D Position, float Rotation, Vector2D Scale)
      : Position(Position), Scale(Scale), Rotation(Rotation) {}

  REPLICATE_COMPONENT(TransformComponent, (Vector2D, Position), (float, Rotation), (Vector2D, Scale))
}
