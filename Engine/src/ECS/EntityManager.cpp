#include "EntityManager.h"
#include "Log.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <limits>

namespace Engine
{
  EntityManager::EntityManager()
  {
    // Reserve memory for performance
    EntityGeneration.reserve(MAX_ENTITIES);
    IsAlive.resize(MAX_ENTITIES, false);
    ComponentMasks.resize(MAX_ENTITIES, 0); // Directly resize and initialize to 0

    // One slot per possible component id, populated lazily the first time that component type
    // is actually used (see AddComponent), so unused component types cost nothing.
    Components.resize(MAX_COMPONENTS);

    // Fill FreeIndices with all possible entity IDs for reuse
    for (size_t EntityIndex = 0; EntityIndex < MAX_ENTITIES; ++EntityIndex)
    {
      EntityGeneration.push_back(0);      // All generations start at 0
      FreeIndices.push_back(EntityIndex); // Populate FreeIndices with all indices
    }
  }

  // Entity Management
  Engine::Entity EntityManager::CreateEntity()
  {
    if (FreeIndices.empty())
    {
      throw std::runtime_error("No more entities can be created! Maximum entity limit reached.");
    }

    // Retrieve the next available entity index
    uint32_t EntityIndex = FreeIndices.front();
    FreeIndices.pop_front();

    IsAlive[EntityIndex] = true;

    // Return a new entity with the current generation
    return MakeEntity(EntityIndex, EntityGeneration[EntityIndex]);
  }

  void EntityManager::DestroyEntity(Engine::Entity Entity)
  {
    const uint32_t EntityIndex = Entity.Index();

    // Ensure the index is valid
    if (EntityIndex >= MAX_ENTITIES)
    {
      throw std::invalid_argument("Invalid entity index in DestroyEntity!");
    }

    // Increment generation to invalidate existing references, wrapping around with modulo
    EntityGeneration[EntityIndex] = (EntityGeneration[EntityIndex] + 1) % (std::numeric_limits<uint8_t>::max() + 1);

    IsAlive[EntityIndex] = false;
    ComponentMasks[EntityIndex].reset(); // Clear all component flags so a recycled index doesn't
                                         // inherit the previous occupant's components

    // Recycle the index
    FreeIndices.push_back(EntityIndex);
  }

  bool EntityManager::IsEntityAlive(Engine::Entity Entity) const
  {
    return EntityGeneration[Entity.Index()] == Entity.Generation();
  }

  // System Management
  void EntityManager::RegisterSystem(SystemFunc System, int32_t Priority)
  {
    // Not an error: two systems sharing a priority is well-defined (registration order breaks
    // the tie), and sometimes genuinely fine (order between them truly doesn't matter). Still
    // worth a warning, since it's just as often an accidental reuse of a priority already claimed
    // by e.g. Engine::SystemPriority::Animation.
    bool PriorityAlreadyUsed = std::any_of(Systems.begin(), Systems.end(),
                                           [Priority](const std::pair<int32_t, SystemFunc> &Existing)
                                           { return Existing.first == Priority; });
    if (PriorityAlreadyUsed)
    {
      ENGINE_LOG_WARN("RegisterSystem: priority %d is already used by another system; order "
                       "between them falls back to registration order",
                       Priority);
    }

    Systems.emplace_back(Priority, std::move(System));

    // Re-sort on every registration rather than a one-shot sort in RunSystems: stable_sort only
    // ever reorders relative to differing priorities, so this correctly preserves registration
    // order among equal priorities across separate RegisterSystem calls, not just within one.
    std::stable_sort(Systems.begin(), Systems.end(),
                      [](const std::pair<int32_t, SystemFunc> &A, const std::pair<int32_t, SystemFunc> &B)
                      { return A.first < B.first; });
  }

  void EntityManager::RunSystems(float DeltaTime)
  {
    for (std::pair<int32_t, SystemFunc> &System : Systems)
    {
      System.second(DeltaTime); // Run all registered systems, in priority order
    }
  }

  Engine::Entity EntityManager::MakeEntity(uint32_t EntityIndex, uint8_t Generation)
  {
    Engine::Entity Entity;
    Entity.Id = (Generation << ENTITY_INDEX_BITS) | EntityIndex;
    return Entity;
  }
}
