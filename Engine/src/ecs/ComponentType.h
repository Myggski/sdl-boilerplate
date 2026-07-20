#pragma once

#include "Core.h"
#include <cstdint>
#include <typeindex>

namespace Engine
{
  using ComponentId = uint8_t;

  namespace ComponentTypeDetail
  {
    // Looks up (or assigns on first use) the id for a component type by RTTI identity, so the
    // same type always maps to the same id no matter whether Engine or Game code asks for it
    // first. A plain "static counter inside a template function" wouldn't work here: component
    // types can be used from both sides of the Engine/Game DLL boundary, and each binary gets
    // its own copy of a template-local static, so a counter alone could hand out different ids
    // for the same type depending on which binary happened to ask first. Routing the actual
    // allocation through this single exported function (it lives in the Engine DLL) keeps it
    // globally unique.
    ENGINE_API ComponentId GetOrAssignComponentId(std::type_index Type);
  }

  template <typename T>
  ComponentId GetComponentId()
  {
    static const ComponentId Id = ComponentTypeDetail::GetOrAssignComponentId(std::type_index(typeid(T)));
    return Id;
  }
}
