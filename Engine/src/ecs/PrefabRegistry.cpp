#include "PrefabRegistry.h"
#include "Log.h"
#include "component/NetworkIdComponent.h"
#include "component/PrefabComponent.h"
#include <unordered_map>

namespace Engine
{
  namespace
  {
    std::unordered_map<uint16_t, PrefabSpawnFn> &Registry()
    {
      static std::unordered_map<uint16_t, PrefabSpawnFn> Table;
      return Table;
    }
  }

  void RegisterPrefab(uint16_t PrefabId, PrefabSpawnFn SpawnFn)
  {
    auto &Table = Registry();
    if (Table.find(PrefabId) != Table.end())
    {
      ENGINE_LOG_ERROR("PrefabRegistry: prefab id %u already registered - two REGISTER_PREFAB "
                        "names hash to the same id, rename one", PrefabId);
    }
    Table[PrefabId] = SpawnFn;
  }

  std::optional<Entity> SpawnPrefab(EngineContext &Context, uint16_t PrefabId, uint32_t NetworkId, uint32_t OwnerConnectionId)
  {
    auto &Table = Registry();
    auto It = Table.find(PrefabId);
    if (It == Table.end())
    {
      ENGINE_LOG_ERROR("PrefabRegistry: no spawn function registered for prefab id %u", PrefabId);
      return std::nullopt;
    }

    Entity NewEntity = It->second(Context);
    Context.World.AddComponent<NetworkIdComponent>(NewEntity, {NetworkId, OwnerConnectionId});
    Context.World.AddComponent<PrefabComponent>(NewEntity, {PrefabId});
    return NewEntity;
  }
}
