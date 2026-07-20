#pragma once

#include "Core.h"
#include "RPC.h"
#include "EntityManager.h"
#include <cstdint>
#include <vector>

namespace Engine
{
  struct ReplicatedComponentInfo
  {
    uint16_t TypeTag; // HashName(#ComponentType) - must be deterministic across processes, unlike
                       // ComponentType.h's GetComponentId, so never put that on the wire.
    bool (*HasComponent)(EntityManager &, Entity);
    void (*Serialize)(EntityManager &, Entity, std::vector<uint8_t> &);
    void (*Apply)(EntityManager &, Entity, const std::vector<uint8_t> &, size_t &);
  };

  ENGINE_API void RegisterReplicatedComponent(ReplicatedComponentInfo Info);

  // (TypeTag, ByteLength, bytes)* for every registered type Target currently has.
  ENGINE_API std::vector<uint8_t> BuildEntitySnapshot(EntityManager &World, Entity Target);

  // An unrecognized TypeTag (build skew) is skipped, not an error - the loop advances by
  // ByteLength regardless, so one bad chunk can't desync the rest of the buffer.
  ENGINE_API void ApplyEntitySnapshot(EntityManager &World, Entity Target, const std::vector<uint8_t> &Buffer);
}

// REPLICATE_COMPONENT(ComponentType, (Type1, Field1), ...) makes a component part of every
// entity's replication snapshot. ComponentType must be default-constructible with public fields;
// fields are read back by name, not positionally, so listed order doesn't need to match a
// constructor. Place this in the component's .cpp, never its header.
#define REPLICATE_WRITE_STMT(Pair) REPLICATE_WRITE_STMT_ Pair
#define REPLICATE_WRITE_STMT_(Type, Name) Engine::RPC::WriteField(Buffer__, Component__->Name);

#define REPLICATE_APPLY_STMT(Pair) REPLICATE_APPLY_STMT_ Pair
#define REPLICATE_APPLY_STMT_(Type, Name) Component__.Name = Engine::RPC::ReadField<Type>(Buffer__, Offset__);

#define REPLICATE_COMPONENT(ComponentType, ...)                                                           \
  namespace                                                                                                \
  {                                                                                                         \
    struct ComponentType##_ReplicationRegistrar                                                            \
    {                                                                                                        \
      ComponentType##_ReplicationRegistrar()                                                                 \
      {                                                                                                       \
        Engine::RegisterReplicatedComponent(Engine::ReplicatedComponentInfo{                                  \
            Engine::RPC::HashName(#ComponentType),                                                            \
            [](Engine::EntityManager &World__, Engine::Entity Target__) -> bool                              \
            { return World__.GetComponent<ComponentType>(Target__) != nullptr; },                            \
            [](Engine::EntityManager &World__, Engine::Entity Target__, std::vector<uint8_t> &Buffer__)      \
            {                                                                                                 \
              ComponentType *Component__ = World__.GetComponent<ComponentType>(Target__);                    \
              RPC_FOR_EACH(REPLICATE_WRITE_STMT, __VA_ARGS__)                                                 \
            },                                                                                                \
            [](Engine::EntityManager &World__, Engine::Entity Target__,                                       \
               const std::vector<uint8_t> &Buffer__, size_t &Offset__)                                        \
            {                                                                                                 \
              ComponentType Component__{};                                                                    \
              RPC_FOR_EACH(REPLICATE_APPLY_STMT, __VA_ARGS__)                                                  \
              World__.AddComponent<ComponentType>(Target__, Component__);                                     \
            }});                                                                                               \
      }                                                                                                        \
    } ComponentType##_ReplicationRegistrarInstance;                                                            \
  }
