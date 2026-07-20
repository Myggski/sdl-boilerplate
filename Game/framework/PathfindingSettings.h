#pragma once

// Only the one header actually needed here, not the whole Engine.h umbrella: see
// CollisionSettings.h's comment for why (EntryPoint.h/main() duplication risk).
#include "src/pathfinding/NavGrid.h"
#include "CollisionSettings.h"

namespace Game
{
  // Which layers a NavGrid rebuild treats as pathing obstacles. DeployableWall traps block too
  // despite being dynamic - ColliderComponent::IsStatic isn't the signal here. NPCs are excluded:
  // avoiding other moving agents is a local-avoidance concern, not the nav grid's.
  inline Engine::PathBlockingLayers BlockingLayers{Layers::Terrain | Layers::DeployableWall};
}
