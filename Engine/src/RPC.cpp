#include "RPC.h"
#include "Log.h"
#include <unordered_map>

namespace Engine::RPC
{
  namespace
  {
    std::unordered_map<uint16_t, HandlerFn> &DispatchTable()
    {
      static std::unordered_map<uint16_t, HandlerFn> Table;
      return Table;
    }
  }

  void RegisterHandler(uint16_t WireId, HandlerFn Fn)
  {
    auto &Table = DispatchTable();
    if (Table.find(WireId) != Table.end())
    {
      ENGINE_LOG_ERROR("RPC: wire id %u already registered - two RPC_SERVER/RPC_CLIENT names hash to the same id, rename one", WireId);
    }
    Table[WireId] = std::move(Fn);
  }

  void Dispatch(EngineContext &Context, uint32_t Sender, const std::vector<uint8_t> &Payload)
  {
    if (Payload.size() < sizeof(uint16_t))
    {
      return;
    }
    uint16_t WireId;
    std::memcpy(&WireId, Payload.data(), sizeof(WireId));

    auto &Table = DispatchTable();
    auto It = Table.find(WireId);
    if (It != Table.end())
    {
      It->second(Context, Sender, Payload);
    }
    else
    {
      ENGINE_LOG_WARN("RPC: no handler registered for wire id %u", WireId);
    }
  }
}
