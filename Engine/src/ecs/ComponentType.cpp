#include "ComponentType.h"
#include <unordered_map>

namespace Engine::ComponentTypeDetail
{
  ComponentId GetOrAssignComponentId(std::type_index Type)
  {
    static std::unordered_map<std::type_index, ComponentId> Ids;
    static ComponentId NextId = 0;

    auto Iterator = Ids.find(Type);
    if (Iterator != Ids.end())
    {
      return Iterator->second;
    }

    ComponentId NewId = NextId++;
    Ids.emplace(Type, NewId);
    return NewId;
  }
}
