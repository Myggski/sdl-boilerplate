#pragma once

#include "Core.h"
#include "EngineContext.h"
#include "EntityManager.h"
#include "component/NetworkIdComponent.h"
#include <cstdint>
#include <optional>

namespace Engine
{
  // Host-only (the caller's responsibility - nothing here enforces it): allocates the next
  // NetworkId. Monotonic, never reused, never 0, unrelated to connection ids - see
  // NetworkIdComponent's own comment for why a networked entity's identity can't just reuse its
  // owning connection's id (not every networked entity has one).
  ENGINE_API uint32_t AllocateNetworkId();

  // Linear scan over NetworkIdComponent - fine at this project's scale (a handful of networked
  // entities) and at RPC-handling cadence (not a hot per-frame path), and avoids a second,
  // separately-maintained index that could drift out of sync with the component data itself.
  // Entity{} has no valid "not found" sentinel (see EntityManager/Entity.h), hence optional.
  ENGINE_API std::optional<Entity> FindEntityByNetworkId(EntityManager &World, uint32_t NetworkId);

  // Returns the *first* entity owned by OwnerConnectionId. Fine today - each connection owns
  // exactly one entity, its player. Once a connection can own more than one (a player and their
  // own placed trap, say), a caller needing a specific one will need an additional filter beyond
  // this - not a problem to solve until that's a real case.
  ENGINE_API std::optional<Entity> FindEntityByOwner(EntityManager &World, uint32_t OwnerConnectionId);

  // Sender is trustworthy (transport-verified, see NetworkManager); NetworkId identifies the
  // entity being acted on. True if Sender is the host, or genuinely owns that entity - false
  // (including if no such entity exists at all).
  ENGINE_API bool HasAuthorityOverEntity(EngineContext &Context, uint32_t Sender, uint32_t NetworkId);
}
