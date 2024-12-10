#include "Core.h"
#include <any>
#include <vector>
#include <iostream>
#include <optional>

namespace Engine
{
  class ComponentStorage
  {
  public:
    ComponentStorage() = default;
    ComponentStorage(size_t MaxNumberOfEntities)
    {
      ComponentData.resize(MaxNumberOfEntities);

      for (std::optional<std::any> &Data : ComponentData)
      {
        Data = std::nullopt; // Set each element to empty
      }
    }

    template <typename T>
    void AddComponent(Engine::Entity Entity, const T &Component)
    {
      if (Entity.Index() < 0 || Entity.Index() >= ComponentData.size())
      {
        std::cout << Entity.Index() << std::endl;
        throw std::out_of_range("EntityId is out of bounds.");
      }

      ComponentData[Entity.Index()] = std::make_any<T>(std::forward<const T>(Component));
    }

    template <typename T>
    T *GetComponent(Engine::Entity Entity)
    {
      if (Entity.Index() >= 0 && Entity.Index() < ComponentData.size()) // Check if the index is valid
      {
        auto &Component = ComponentData[Entity.Index()];
        if (Component.has_value())
        {
          return std::any_cast<T>(&Component.value());
        }
      }
      return nullptr;
    }

    void RemoveComponent(Engine::Entity Entity)
    {
      if (Entity.Index() >= 0 && Entity.Index() < ComponentData.size())
      {
        ComponentData[Entity.Index()].reset(); // Clear the value for this entity
      }
    }

  private:
    std::vector<std::optional<std::any>> ComponentData; // Fixed-size storage
  };
}
