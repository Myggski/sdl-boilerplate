#include "PathFollowSystem.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/VelocityComponent.h"
#include "src/ecs/component/PathFollowComponent.h"

namespace Engine
{
  void PathFollowSystem(EntityManager &World, float)
  {
    World.ForEach<TransformComponent, VelocityComponent, PathFollowComponent>(
        [](Entity, TransformComponent &Transform, VelocityComponent &Velocity, PathFollowComponent &Follow)
        {
          if (Follow.HasArrived || Follow.Waypoints.empty())
          {
            Velocity.X = 0.0f;
            Velocity.Y = 0.0f;
            return;
          }

          // Loop, not a single check: advances through every waypoint already within
          // ArrivalRadius in one tick, rather than stalling one tick per waypoint if DeltaTime
          // or Speed ever let an entity cross more than one waypoint's radius at once.
          while (Follow.CurrentIndex < Follow.Waypoints.size() &&
                 Transform.Position.DistanceSquared(Follow.Waypoints[Follow.CurrentIndex]) <=
                     Follow.ArrivalRadius * Follow.ArrivalRadius)
          {
            ++Follow.CurrentIndex;
          }

          if (Follow.CurrentIndex >= Follow.Waypoints.size())
          {
            Follow.HasArrived = true;
            Velocity.X = 0.0f;
            Velocity.Y = 0.0f;
            return;
          }

          Vector2D Direction = (Follow.Waypoints[Follow.CurrentIndex] - Transform.Position).Normalized();
          Velocity.X = Direction.X * Follow.Speed;
          Velocity.Y = Direction.Y * Follow.Speed;
        });
  }
}
