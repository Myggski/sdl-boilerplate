#pragma once

#include "Core.h"
#include <bitset>
#include <vector>
#include <queue>
#include <functional>
#include <cstring>
#include "Entity.h"
#include "ComponentStorage.h"

namespace Engine
{
  // Constants
  constexpr size_t MAX_COMPONENTS = 32;

  // Type Definitions
  using ComponentMask = std::bitset<MAX_COMPONENTS>;
  using SystemFunc = std::function<void(float)>;

  // ECS Engine
  class ENGINE_API EntityManager
  {
  public:
    EntityManager();

    // Entity Management
    Engine::Entity CreateEntity();
    void DestroyEntity(Engine::Entity Entity);
    bool IsEntityAlive(Engine::Entity Entity) const;

    // Component Management
    void RemoveComponent(Engine::Entity Entity, int ComponentId);

    template <typename T>
    void AddComponent(Engine::Entity Entity, int ComponentId, const T &Component)
    {
      if (!IsEntityAlive(Entity))
      {
        throw std::runtime_error("Cannot add component to inactive entity!");
      }

      if (ComponentId < 0 || ComponentId >= MAX_COMPONENTS)
      {
        throw std::invalid_argument("Invalid ComponentId.");
      }

      // Add the component to the storage
      Components[ComponentId].AddComponent<T>(Entity, std::forward<const T>(Component));

      // Update the component mask
      ComponentMasks[Entity.Index()].set(ComponentId);
    }

    template <typename T>
    T *GetComponent(Engine::Entity Entity, int32_t ComponentId)
    {
      if (!IsEntityAlive(Entity) || !ComponentMasks[Entity.Index()].test(ComponentId))
      {
        return nullptr;
      }

      // Retrieve the component from storage
      return Components[ComponentId].GetComponent<T>(Entity);
    }

    // System Management
    void RegisterSystem(SystemFunc System);
    void RunSystems(float DeltaTime);

  private:
    Engine::Entity MakeEntity(uint32_t EntityIndex, uint8_t Generation);

  private:
    // Internal Data
    std::vector<uint8_t> EntityGeneration;     // Tracks the generation for each entity slot
    std::deque<uint32_t> FreeIndices;          // Queue of recycled indices
    std::vector<ComponentMask> ComponentMasks; // Component masks
    std::vector<SystemFunc> Systems;           // Registered systems
    std::vector<ComponentStorage> Components;  // Component storage array
  };
}