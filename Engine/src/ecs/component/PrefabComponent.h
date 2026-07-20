#pragma once

#include "Core.h"
#include <cstdint>

namespace Engine
{
  // Attached alongside NetworkIdComponent by SpawnPrefab (PrefabRegistry.h).
  struct ENGINE_API PrefabComponent
  {
    uint16_t PrefabId = 0;
  };
}
