#pragma once

#include "Core.h"
#include <any>
#include <bitset>
#include <cstdint>
#include <deque>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <utility>
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

        // Fetch each queried component pointer once (not once to check presence, then again to
        // build the callback args): GetComponent already does a bitset test + any_cast per call,
        // no reason to pay for that twice per entity per queried type.
        std::tuple<QueryComponents *...> Found{GetComponent<QueryComponents>(Candidate)...};
        if ((std::get<QueryComponents *>(Found) && ...))
        {
          Callback(Candidate, (*std::get<QueryComponents *>(Found))...);
        }
      }
    }

    // System Management. Priority controls run order within RunSystems, lower runs first, stable
    // (equal priorities keep registration order). See Engine::SystemPriority for the priorities
    // EngineContext auto-registers Movement/Animation/Collision at; a game inserting its own
    // system should pick a priority relative to those constants (e.g. SystemPriority::Movement -
    // 1 to run before movement) rather than a guessed number.
    void RegisterSystem(SystemFunc System, int32_t Priority = 0);
    void RunSystems(float DeltaTime);

    // Resource Management: a single global instance per type, not tied to any entity. This is how
    // systems pass data to each other (e.g. this tick's collision results) without needing a
    // return value, so they can stay plain (EntityManager&, float) functions registrable via
    // RegisterSystem like any other system. Not collision-specific; reusable for anything a later
    // system needs to publish for others to read the same tick (input state, score, ...).
    template <typename T>
    void SetResource(T Value)
    {
      Resources[std::type_index(typeid(T))] = std::move(Value);
    }

    template <typename T>
    T *GetResource()
    {
      auto It = Resources.find(std::type_index(typeid(T)));
      return It != Resources.end() ? std::any_cast<T>(&It->second) : nullptr;
    }

  private:
    Engine::Entity MakeEntity(uint32_t EntityIndex, uint8_t Generation);

  private:
    // Internal Data
    std::vector<uint8_t> EntityGeneration;     // Tracks the generation for each entity slot
    std::vector<bool> IsAlive;                 // Tracks which indices are currently in use, for ForEach
    std::deque<uint32_t> FreeIndices;          // Queue of recycled indices
    std::vector<ComponentMask> ComponentMasks; // Component masks
    std::vector<std::pair<int32_t, SystemFunc>> Systems; // Registered systems, kept sorted by priority
    std::vector<std::any> Components;          // One ComponentArray<T> per component type, indexed by ComponentId
    std::unordered_map<std::type_index, std::any> Resources; // One instance per type, see SetResource/GetResource
  };
}
