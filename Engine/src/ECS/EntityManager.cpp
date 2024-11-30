#include "EntityManager.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace Engine
{
  EntityManager::EntityManager()
  {
    // Initialize EntityAlive and ComponentMasks
    EntityAlive.reserve(MAX_ENTITIES);
    ComponentMasks.reserve(MAX_ENTITIES); // Resize to MAX_ENTITIES and initialize each component mask to zero

    // Fill AvailableEntities with all possible entity IDs for reuse
    for (EntityId i = 0; i < MAX_ENTITIES; ++i)
    {
      EntityAlive.push_back(false);
      AvailableEntities.push(i); // Push each entity ID into the available queue
      ComponentMasks.push_back(0);
    }
  }

  // Entity Management
  EntityId EntityManager::CreateEntity()
  {
    if (AvailableEntities.empty())
    {
      throw std::runtime_error("No more entities available!");
    }
    EntityId Entity = AvailableEntities.front();
    AvailableEntities.pop();
    EntityAlive[Entity] = true;
    return Entity;
  }

  void EntityManager::DestroyEntity(EntityId Entity)
  {
    if (!IsEntityAlive(Entity))
    {
      return;
    }
    EntityAlive[Entity] = false;
    ComponentMasks[Entity].reset(); // Clear all components for the entity
    AvailableEntities.push(Entity); // Recycle the entity ID
  }

  bool EntityManager::IsEntityAlive(EntityId Entity) const
  {
    return Entity >= 0 && Entity < MAX_ENTITIES && EntityAlive[Entity];
  }

  // Component Management
  void EntityManager::AddComponent(EntityId Entity, int ComponentId, void *Data, size_t Size)
  {
    if (!IsEntityAlive(Entity))
    {
      throw std::runtime_error("Cannot add component to inactive entity!");
    }

    if (ComponentId < 0 || ComponentId >= MAX_COMPONENTS)
    {
      throw std::invalid_argument("Invalid ComponentId.");
    }

    // Ensure the component data array is initialized
    if (Components[ComponentId].Data == nullptr)
    {
      Components[ComponentId].Data = std::malloc(MAX_ENTITIES * Size); // Allocate memory for MAX_ENTITIES components
      Components[ComponentId].Size = Size;
      std::memset(Components[ComponentId].Data, 0, MAX_ENTITIES * Size); // Initialize with zeroes
    }

    ComponentMasks[Entity].set(ComponentId);                                                    // Set the bit for this component
    std::memcpy(static_cast<char *>(Components[ComponentId].Data) + Entity * Size, Data, Size); // Copy the data into the correct location
  }

  void EntityManager::RemoveComponent(EntityId Entity, int ComponentId)
  {
    if (!IsEntityAlive(Entity))
      return;

    if (ComponentId < 0 || ComponentId >= MAX_COMPONENTS)
      return;

    ComponentMasks[Entity].reset(ComponentId); // Remove the component by resetting the bit
  }

  void *EntityManager::GetComponent(EntityId Entity, int ComponentId)
  {
    if (!IsEntityAlive(Entity) || !ComponentMasks[Entity].test(ComponentId))
    {
      return nullptr; // Return nullptr if the entity doesn't have the requested component
    }

    return static_cast<char *>(Components[ComponentId].Data) + Entity * Components[ComponentId].Size; // Return the component data
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
}