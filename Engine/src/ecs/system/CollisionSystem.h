#pragma once

#include "Core.h"
#include "src/ecs/Entity.h"
#include <vector>

namespace Engine
{
  class EntityManager;

  struct ENGINE_API CollisionPair
  {
    Entity A;
    Entity B;
  };

  // Published each call via World.SetResource<CollisionResults>(...), read it back with
  // World.GetResource<CollisionResults>() from any system registered after this one.
  struct ENGINE_API CollisionResults
  {
    std::vector<CollisionPair> Pairs;
  };

  // Broad-phase (uniform spatial grid, cell size CellSize) plus narrow-phase (AABB-AABB,
  // Circle-Circle, AABB-Circle) detection over every entity with a Transform + Collider, filtered
  // by World.GetResource<CollisionMatrix>()->DoLayersCollide(A.Layer, B.Layer) (see
  // CollisionMatrix.h: colliders only carry their own Layer, never a mask). No matrix resource
  // set means nothing is checked against anything (a game publishes its own matrix via
  // World.SetResource<CollisionMatrix>(...), typically once at startup). Detection only, no
  // resolution or pushback: reacting to a collision (damage, pickups, ...) is inherently
  // game-specific, so that's left to game-side systems reading CollisionResults.
  //
  // Auto-registered by EngineContext at SystemPriority::Collision, same as
  // MovementSystem/AnimationSystem, since every game needs this and shouldn't have to remember to
  // register it. It can't simply return its results the way a plain function would, though:
  // RegisterSystem's std::function<void(float)> has no way to carry a return value back out, so
  // results are published as the CollisionResults resource instead.
  ENGINE_API void CollisionSystem(EntityManager &World, float DeltaTime, float CellSize = 64.0f);
}
