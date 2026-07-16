#pragma once

#include "Core.h"
#include <any>
#include <bitset>
#include <deque>
#include <vector>
#include <functional>
#include "Entity.h"
#include "ComponentArray.h"
#include "ComponentType.h"

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
    template <typename T>
    void AddComponent(Engine::Entity Entity, const T &Component)
    {
      if (!IsEntityAlive(Entity))
      {
        throw std::runtime_error("Cannot add component to inactive entity!");
      }

      ComponentId Id = GetComponentId<T>();
      if (Id >= MAX_COMPONENTS)
      {
        throw std::invalid_argument("Exceeded MAX_COMPONENTS distinct component types.");
      }

      if (!Components[Id].has_value())
      {
        Components[Id] = ComponentArray<T>(MAX_ENTITIES);
      }

      std::any_cast<ComponentArray<T> &>(Components[Id]).Set(Entity, Component);
      ComponentMasks[Entity.Index()].set(Id);
    }

    template <typename T>
    T *GetComponent(Engine::Entity Entity)
    {
      ComponentId Id = GetComponentId<T>();
      if (!IsEntityAlive(Entity) || Id >= MAX_COMPONENTS || !ComponentMasks[Entity.Index()].test(Id))
      {
        return nullptr;
      }

      return &std::any_cast<ComponentArray<T> &>(Components[Id]).Get(Entity);
    }

    template <typename T>
    void RemoveComponent(Engine::Entity Entity)
    {
      if (!IsEntityAlive(Entity))
      {
        return;
      }

      ComponentId Id = GetComponentId<T>();
      if (Id >= MAX_COMPONENTS)
      {
        return;
      }

      ComponentMasks[Entity.Index()].reset(Id);
    }

    // Calls Callback(Entity, T1&, T2&, ...) for every alive entity that has all of Components...
    // e.g. Manager.ForEach<Position, Velocity>([](Entity E, Position &P, Velocity &V) { ... });
    template <typename... QueryComponents, typename Func>
    void ForEach(Func &&Callback)
    {
      for (uint32_t Index = 0; Index < MAX_ENTITIES; ++Index)
      {
        if (!IsAlive[Index])
        {
          continue;
        }

        Engine::Entity Candidate = MakeEntity(Index, EntityGeneration[Index]);
        if ((GetComponent<QueryComponents>(Candidate) && ...))
        {
          Callback(Candidate, (*GetComponent<QueryComponents>(Candidate))...);
        }
      }
    }

    // System Management
    void RegisterSystem(SystemFunc System);
    void RunSystems(float DeltaTime);

  private:
    Engine::Entity MakeEntity(uint32_t EntityIndex, uint8_t Generation);

  private:
    // Internal Data
    std::vector<uint8_t> EntityGeneration;     // Tracks the generation for each entity slot
    std::vector<bool> IsAlive;                 // Tracks which indices are currently in use, for ForEach
    std::deque<uint32_t> FreeIndices;          // Queue of recycled indices
    std::vector<ComponentMask> ComponentMasks; // Component masks
    std::vector<SystemFunc> Systems;           // Registered systems
    std::vector<std::any> Components;          // One ComponentArray<T> per component type, indexed by ComponentId
  };
}
