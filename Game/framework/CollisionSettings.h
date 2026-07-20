#pragma once

// Only the one header actually needed here, not the whole Engine.h umbrella: that pulls in
// EntryPoint.h, which defines main() inline. Fine included from exactly one .cpp (Game.cpp), but
// a second one including it too would duplicate main() at link time.
#include "src/ecs/CollisionMatrix.h"

namespace Game
{
  // Collision layers are game-specific, so Engine only exposes a raw Layer bit per collider.
  // This is where a game defines what they actually mean.
  namespace Layers
  {
    constexpr uint32_t Player = 1u << 0;
    constexpr uint32_t Terrain = 1u << 1;
    constexpr uint32_t Prop = 1u << 2;

    // A movable/toggleable wall-style trap: physically solid like Terrain, but dynamic (not
    // IsStatic), so it also needs its own layer bit for PathfindingSettings.h's blocking mask,
    // which can't key off IsStatic (see that file's comment for why).
    constexpr uint32_t DeployableWall = 1u << 3;
  }

  // Which layers interact, declared once per pair, symmetrically, see
  // Engine/src/ecs/CollisionMatrix.h. This is the one place to edit a relationship; colliders
  // only ever specify their own Layer. `inline` (not `extern` + a matching .cpp): a C++17 inline
  // variable can be defined directly in a header and included from multiple translation units
  // without an ODR violation, the same guarantee `inline` functions have always had.
  //
  // Published to CollisionSystem via Context.World.SetResource<CollisionMatrix>(Matrix) in
  // Game::Startup. CollisionSystem is auto-registered by EngineContext, so it can't take this as
  // a direct function argument the way an explicitly-called function could.
  inline Engine::CollisionMatrix Matrix({
      {Layers::Player, Layers::Terrain},
      {Layers::Player, Layers::Prop},
      {Layers::Terrain, Layers::Prop},
      {Layers::Player, Layers::DeployableWall},
      {Layers::Prop, Layers::DeployableWall},
  });
}
