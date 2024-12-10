#include "EntityManager.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace Engine
{
  EntityManager::EntityManager()
  {
    // Reserve memory for performance
    EntityGeneration.reserve(MAX_ENTITIES);
    ComponentMasks.resize(MAX_ENTITIES, 0); // Directly resize and initialize to 0
    Components.reserve(MAX_COMPONENTS);

    // Initialize component storage for each potential component type
    for (int32_t ComponentId = 0; ComponentId < MAX_COMPONENTS; ++ComponentId)
    {
      Components.push_back(ComponentStorage{MAX_ENTITIES});
    }

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

    // Recycle the index
    FreeIndices.push_back(EntityIndex);
  }

  bool EntityManager::IsEntityAlive(Engine::Entity Entity) const
  {
    return EntityGeneration[Entity.Index()] == Entity.Generation();
  }

  void EntityManager::RemoveComponent(Engine::Entity Entity, int ComponentId)
  {
    if (!IsEntityAlive(Entity))
      return;

    if (ComponentId < 0 || ComponentId >= MAX_COMPONENTS)
      return;

    // Remove the component from storage
    Components[ComponentId].RemoveComponent(Entity);

    // Update the component mask
    ComponentMasks[Entity.Index()].reset(ComponentId);
  }

  // System Management
  void EntityManager::RegisterSystem(SystemFunc System)
  {
    Systems.push_back(System); // Add the system to the list
  }

  void EntityManager::RunSystems(float DeltaTime)
  {
    for (SystemFunc &System : Systems)
    {
      System(DeltaTime); // Run all registered systems
    }
  }

  Engine::Entity EntityManager::MakeEntity(uint32_t EntityIndex, uint8_t Generation)
  {
    Engine::Entity Entity;
    Entity.Id = (Generation << ENTITY_INDEX_BITS) | EntityIndex;
    return Entity;
  }
}