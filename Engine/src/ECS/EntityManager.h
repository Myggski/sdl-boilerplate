#pragma once

#include "Core.h"
#include <bitset>
#include <vector>
#include <queue>
#include <functional>
#include <cstring>

namespace Engine
{
  // Constants
  constexpr size_t MAX_ENTITIES = 5120;
  constexpr size_t MAX_COMPONENTS = 32;

  // Type Definitions
  using EntityId = int32_t;
  using ComponentMask = std::bitset<MAX_COMPONENTS>;
  using SystemFunc = std::function<void(float)>;

  // ECS Engine
  class ENGINE_API EntityManager
  {
  public:
    EntityManager();

    // Entity Management
    EntityId CreateEntity();
    void DestroyEntity(EntityId Entity);
    bool IsEntityAlive(EntityId Entity) const;

    // Component Management
    void AddComponent(EntityId Entity, int ComponentId, void *Data, size_t Size);
    void RemoveComponent(EntityId Entity, int ComponentId);
    void *GetComponent(EntityId Entity, int ComponentId);

    // System Management
    void RegisterSystem(SystemFunc System);
    void RunSystems(float DeltaTime);

  private:
    // Internal Data
    std::vector<bool> EntityAlive;             // Tracks active entities
    std::vector<ComponentMask> ComponentMasks; // Component masks
    std::queue<EntityId> AvailableEntities;    // Recycled entity IDs
    std::vector<SystemFunc> Systems;           // Registered systems

    // Component Storage
    struct ComponentData
    {
    public:
      void *Data;  // Pointer to array of component data
      size_t Size; // Size of each component
    };

    ComponentData Components[MAX_COMPONENTS] = {}; // Component storage array
  };

}