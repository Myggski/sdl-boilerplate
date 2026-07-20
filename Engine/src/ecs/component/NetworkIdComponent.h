#pragma once

#include "Core.h"
#include <cstdint>

namespace Engine
{
  // Attached to any entity that's replicated across the network - players today, later
  // traps/NPCs/whatever else. NetworkId is a host-allocated, connection-independent identity
  // (see AllocateNetworkId in NetworkReplication.h): unlike a connection id, it exists for
  // exactly as long as the entity does, and plenty of networked entities (a host-authoritative
  // trap) have no owning connection at all. OwnerConnectionId is 0 for those - a real
  // client-owned entity (a player) has its owner's connection id here instead, used by
  // HasAuthorityOverEntity.
  struct ENGINE_API NetworkIdComponent
  {
    uint32_t NetworkId = 0;
    uint32_t OwnerConnectionId = 0;
  };
}
