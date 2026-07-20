#include "MovementSystem.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/VelocityComponent.h"

namespace Engine
{
  void MovementSystem(EntityManager &World, float DeltaTime)
  {
    World.ForEach<TransformComponent, VelocityComponent>(
        [DeltaTime](Entity, TransformComponent &Transform, VelocityComponent &Velocity)
        {
          Transform.Position.X += Velocity.X * DeltaTime;
          Transform.Position.Y += Velocity.Y * DeltaTime;
        });
  }
}
