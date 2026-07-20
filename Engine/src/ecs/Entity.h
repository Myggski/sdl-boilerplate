#pragma once

#include <cstdint>
#include <cstddef>

namespace Engine
{
  constexpr size_t MAX_ENTITIES = 1024;
  const uint32_t ENTITY_INDEX_BITS = 22; // Still 22 bits
  const uint32_t ENTITY_INDEX_MASK = (1 << ENTITY_INDEX_BITS) - 1;

  const uint32_t ENTITY_GENERATION_BITS = 8;
  const uint32_t ENTITY_GENERATION_MASK = (1 << ENTITY_GENERATION_BITS) - 1;

  static_assert(ENTITY_INDEX_BITS + ENTITY_GENERATION_BITS <= 32,
                "Total bits for entity index and generation exceed 32!");

  struct ENGINE_API Entity
  {
    Entity() : Id(-1) {}

    uint32_t Id;

    uint32_t Index() const { return Id & ENTITY_INDEX_MASK; }
    uint32_t Generation() const { return (Id >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK; }
  };
}
