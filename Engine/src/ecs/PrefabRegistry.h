#pragma once

#include "Core.h"
#include "EngineContext.h"
#include "EntityManager.h"
#include <cstdint>
#include <optional>

namespace Engine
{
  // No NetworkId/OwnerConnectionId params - SpawnPrefab attaches those centrally below.
  using PrefabSpawnFn = Entity (*)(EngineContext &);

  ENGINE_API void RegisterPrefab(uint16_t PrefabId, PrefabSpawnFn SpawnFn);

  // Runs PrefabId's spawn function, then attaches NetworkIdComponent and PrefabComponent.
  // optional, not bare Entity: Entity{} has no valid "not found" sentinel.
  ENGINE_API std::optional<Entity> SpawnPrefab(EngineContext &Context, uint16_t PrefabId,
                                                uint32_t NetworkId, uint32_t OwnerConnectionId);
}

// REGISTER_PREFAB(UniqueTag, PrefabId, SpawnFn) self-registers SpawnFn under PrefabId at static
// init. PrefabId is typically a named constant from Engine::RPC::HashName("SomeName").
#define REGISTER_PREFAB(UniqueTag, PrefabId, SpawnFn)         \
  namespace                                                   \
  {                                                            \
    struct UniqueTag##_PrefabRegistrar                         \
    {                                                           \
      UniqueTag##_PrefabRegistrar()                             \
      {                                                          \
        Engine::RegisterPrefab((PrefabId), (SpawnFn));           \
      }                                                          \
    } UniqueTag##_PrefabRegistrarInstance;                       \
  }
