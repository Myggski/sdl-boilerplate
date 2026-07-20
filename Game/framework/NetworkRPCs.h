#pragma once

#include "src/EngineContext.h"
#include "src/RPC.h"
#include "src/math/Vector2D.h"
#include <cstdint>
#include <vector>

namespace Game
{
  RPC_DECLARE_SERVER(ServerMovePlayer, (Engine::Vector2D, Target))
  RPC_DECLARE_CLIENT(ClientSpawnNetworkEntity, (uint32_t, NetworkId), (uint16_t, PrefabId),
                      (uint32_t, OwnerConnectionId), (std::vector<uint8_t>, ComponentBlob))
  RPC_DECLARE_CLIENT(ClientSetPath, (uint32_t, NetworkId), (std::vector<Engine::Vector2D>, Waypoints))
  RPC_DECLARE_MULTICAST(MulticastDespawnEntity, (uint32_t, NetworkId))
  RPC_DECLARE_CLIENT(ClientAssignLocalNetworkId, (uint32_t, NetworkId))

  // Host only: broadcasts NetworkId's current path to every client, reconciling by overwrite.
  void BroadcastSetPathFor(Engine::EngineContext &Context, uint32_t NetworkId);
}
