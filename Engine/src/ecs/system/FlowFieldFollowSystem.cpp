#include "FlowFieldFollowSystem.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/VelocityComponent.h"
#include "src/ecs/component/FlowFieldFollowComponent.h"

namespace Engine
{
  void FlowFieldFollowSystem(EntityManager &World, float)
  {
    FlowFieldSet *Set = World.GetResource<FlowFieldSet>();
    if (!Set)
    {
      return;
    }

    World.ForEach<TransformComponent, VelocityComponent, FlowFieldFollowComponent>(
        [Set](Entity, TransformComponent &Transform, VelocityComponent &Velocity, FlowFieldFollowComponent &Follow)
        {
          auto FieldIterator = Set->Fields.find(Follow.GoalId);
          if (FieldIterator == Set->Fields.end())
          {
            Velocity.X = 0.0f;
            Velocity.Y = 0.0f;
            return;
          }

          const FlowField &Field = FieldIterator->second;
          int32_t CellX = Field.WorldToCellX(Transform.Position.X);
          int32_t CellY = Field.WorldToCellY(Transform.Position.Y);

          if (!Field.IsInBounds(CellX, CellY))
          {
            Velocity.X = 0.0f;
            Velocity.Y = 0.0f;
            return;
          }

          Vector2D Direction = Field.Directions[static_cast<size_t>(Field.CellIndex(CellX, CellY))];
          Velocity.X = Direction.X * Follow.Speed;
          Velocity.Y = Direction.Y * Follow.Speed;
        });
  }
}
