#include "NetworkRPCs.h"
#include "NetworkPlayers.h"
#include "src/Log.h"
#include "src/ecs/ComponentReplication.h"
#include "src/ecs/NetworkReplication.h"
#include "src/ecs/PrefabRegistry.h"
#include "src/ecs/component/NetworkIdComponent.h"
#include "src/ecs/component/PathFollowComponent.h"
#include "src/ecs/component/TransformComponent.h"

namespace Game
{
  using namespace Engine;

  // Sender is transport-verified, so this can only ever move the sender's own player.
  RPC_SERVER(ServerMovePlayer, (Vector2D, Target))
  {
    std::optional<Entity> Player = FindEntityByOwner(Context.World, Sender);
    if (!Player)
    {
      return;
    }
    ApplyLocalPathToEntity(Context, *Player, Target);
    NetworkIdComponent *NetId = Context.World.GetComponent<NetworkIdComponent>(*Player);
    BroadcastSetPathFor(Context, NetId->NetworkId);
  }

  RPC_CLIENT(ClientSpawnNetworkEntity, (uint32_t, NetworkId), (uint16_t, PrefabId),
             (uint32_t, OwnerConnectionId), (std::vector<uint8_t>, ComponentBlob))
  {
    if (FindEntityByNetworkId(Context.World, NetworkId))
    {
      return;
    }
    std::optional<Entity> NewEntity = SpawnPrefab(Context, PrefabId, NetworkId, OwnerConnectionId);
    if (!NewEntity)
    {
      return; // unknown PrefabId - SpawnPrefab already logged why
    }
    ApplyEntitySnapshot(Context.World, *NewEntity, ComponentBlob);
    ENGINE_LOG_INFO("Network: spawned network id %u (prefab %u)", NetworkId, PrefabId);
  }

  RPC_CLIENT(ClientSetPath, (uint32_t, NetworkId), (std::vector<Vector2D>, Waypoints))
  {
    std::optional<Entity> Player = FindEntityByNetworkId(Context.World, NetworkId);
    if (Player && !Waypoints.empty())
    {
      size_t WaypointCount = Waypoints.size(); // read before the move below leaves Waypoints empty
      Context.World.AddComponent<PathFollowComponent>(*Player, PathFollowComponent{std::move(Waypoints)});
      ENGINE_LOG_INFO("Network: set path for network id %u, %zu waypoints", NetworkId, WaypointCount);
    }
  }

  RPC_MULTICAST(MulticastDespawnEntity, (uint32_t, NetworkId))
  {
    std::optional<Entity> Player = FindEntityByNetworkId(Context.World, NetworkId);
    if (Player)
    {
      Context.World.DestroyEntity(*Player);
      ENGINE_LOG_INFO("Network: despawned network id %u", NetworkId);
    }
  }

  RPC_CLIENT(ClientAssignLocalNetworkId, (uint32_t, NetworkId))
  {
    Context.World.GetResource<LocalPlayerState>()->NetworkId = NetworkId;
    ENGINE_LOG_INFO("Network: assigned local network id %u", NetworkId);
  }

  void BroadcastSetPathFor(EngineContext &Context, uint32_t NetworkId)
  {
    std::optional<Entity> Player = FindEntityByNetworkId(Context.World, NetworkId);
    if (!Player)
    {
      return;
    }

    PathFollowComponent *Follow = Context.World.GetComponent<PathFollowComponent>(*Player);
    if (!Follow)
    {
      return; // FindPath failed (blocked/unreachable click) - nothing to tell clients
    }

    Engine::RPC::CallAllClients(Context, ClientSetPath, NetworkId, Follow->Waypoints);
  }
}
