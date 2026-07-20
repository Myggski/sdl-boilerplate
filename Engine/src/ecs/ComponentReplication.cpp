#include "ComponentReplication.h"
#include "Log.h"

namespace Engine
{
  namespace
  {
    std::vector<ReplicatedComponentInfo> &Registry()
    {
      static std::vector<ReplicatedComponentInfo> Table;
      return Table;
    }
  }

  void RegisterReplicatedComponent(ReplicatedComponentInfo Info)
  {
    for (const ReplicatedComponentInfo &Existing : Registry())
    {
      if (Existing.TypeTag == Info.TypeTag)
      {
        ENGINE_LOG_ERROR("ComponentReplication: type tag %u already registered - two "
                          "REPLICATE_COMPONENT type names hash to the same id, rename one", Info.TypeTag);
      }
    }
    Registry().push_back(Info);
  }

  std::vector<uint8_t> BuildEntitySnapshot(EntityManager &World, Entity Target)
  {
    std::vector<uint8_t> Buffer;
    for (const ReplicatedComponentInfo &Info : Registry())
    {
      if (!Info.HasComponent(World, Target))
      {
        continue;
      }
      std::vector<uint8_t> Payload;
      Info.Serialize(World, Target, Payload);
      Engine::RPC::WriteField(Buffer, Info.TypeTag);
      Engine::RPC::WriteField(Buffer, static_cast<uint32_t>(Payload.size()));
      Buffer.insert(Buffer.end(), Payload.begin(), Payload.end());
    }
    return Buffer;
  }

  void ApplyEntitySnapshot(EntityManager &World, Entity Target, const std::vector<uint8_t> &Buffer)
  {
    size_t Offset = 0;
    while (Offset + sizeof(uint16_t) + sizeof(uint32_t) <= Buffer.size())
    {
      uint16_t TypeTag = Engine::RPC::ReadField<uint16_t>(Buffer, Offset);
      uint32_t Length = Engine::RPC::ReadField<uint32_t>(Buffer, Offset);
      size_t ChunkEnd = Offset + Length;
      if (ChunkEnd > Buffer.size())
      {
        ENGINE_LOG_ERROR("ComponentReplication: snapshot truncated (tag %u claims %u bytes, %zu left)",
                          TypeTag, Length, Buffer.size() - Offset);
        break;
      }

      for (const ReplicatedComponentInfo &Info : Registry())
      {
        if (Info.TypeTag == TypeTag)
        {
          size_t FieldOffset = Offset;
          Info.Apply(World, Target, Buffer, FieldOffset);
          break;
        }
      }

      Offset = ChunkEnd; // don't trust wherever Apply left FieldOffset
    }
  }
}
