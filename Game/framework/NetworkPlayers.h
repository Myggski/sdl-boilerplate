#pragma once

#include "src/EngineContext.h"
#include "src/RPC.h"
#include "src/Texture.h"
#include "src/ecs/EntityManager.h"
#include "src/math/Vector2D.h"
#include <cstdint>

namespace Game
{
  // This process's own player's NetworkId, as an ECS resource. 0 = not yet known.
  struct LocalPlayerState
  {
    uint32_t NetworkId = 0;
  };

  constexpr uint16_t PlayerPrefabId = Engine::RPC::HashName("Player");

  // Local-only setup (sprite, collider, animation) with default values; the real Position is
  // filled in right after by the replicated component snapshot.
  Engine::Entity SpawnPlayerEntity(Engine::EngineContext &Context);

  void ApplyLocalPathToEntity(Engine::EngineContext &Context, Engine::Entity Target, Engine::Vector2D ClickWorldPos);
}
