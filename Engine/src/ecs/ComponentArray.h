#pragma once

#include "Core.h"
#include "Entity.h"
#include <vector>

namespace Engine
{
  // Tightly-packed storage for one component type, indexed directly by entity index. Whether a
  // slot is actually in use is tracked by EntityManager's ComponentMask, not here, so this stays
  // a plain flat array with no per-slot presence bookkeeping of its own.
  template <typename T>
  class ComponentArray
  {
  public:
    explicit ComponentArray(size_t MaxEntities) : Data(MaxEntities) {}

    void Set(Entity Entity, const T &Value) { Data[Entity.Index()] = Value; }
    T &Get(Entity Entity) { return Data[Entity.Index()]; }

  private:
    std::vector<T> Data;
  };
}
