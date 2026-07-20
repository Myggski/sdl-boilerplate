#include "NetworkReplication.h"

namespace Engine
{
  uint32_t AllocateNetworkId()
  {
    static uint32_t NextId = 1; // never 0 - see NetworkIdComponent's own comment
    return NextId++;
  }

  std::optional<Entity> FindEntityByNetworkId(EntityManager &World, uint32_t NetworkId)
  {
    std::optional<Entity> Result;
    World.ForEach<NetworkIdComponent>([&](Entity Candidate, NetworkIdComponent &NetId)
                                       {
      if (!Result && NetId.NetworkId == NetworkId)
      {
        Result = Candidate;
      } });
    return Result;
  }

  std::optional<Entity> FindEntityByOwner(EntityManager &World, uint32_t OwnerConnectionId)
  {
    std::optional<Entity> Result;
    World.ForEach<NetworkIdComponent>([&](Entity Candidate, NetworkIdComponent &NetId)
                                       {
      if (!Result && NetId.OwnerConnectionId == OwnerConnectionId)
      {
        Result = Candidate;
      } });
    return Result;
  }

  bool HasAuthorityOverEntity(EngineContext &Context, uint32_t Sender, uint32_t NetworkId)
  {
    if (Context.Network.IsHost())
    {
      return true;
    }
    std::optional<Entity> Target = FindEntityByNetworkId(Context.World, NetworkId);
    if (!Target)
    {
      return false;
    }
    NetworkIdComponent *NetId = Context.World.GetComponent<NetworkIdComponent>(*Target);
    return NetId && NetId->OwnerConnectionId == Sender;
  }
}
