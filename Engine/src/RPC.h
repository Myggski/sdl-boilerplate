#pragma once

#include "Core.h"
#include "EngineContext.h"
#include "ecs/NetworkReplication.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <type_traits>
#include <vector>

namespace Engine::RPC
{
  // ---- Serialization: zero registration for any trivially-copyable (POD) type or a
  // std::vector of one. A genuinely new non-POD type needs one hand-written WriteField +
  // FieldReader specialization, added once, anywhere visible via ADL - doesn't have to live here.

  template <typename T>
    requires std::is_trivially_copyable_v<T>
  void WriteField(std::vector<uint8_t> &Buffer, const T &Value)
  {
    const uint8_t *Bytes = reinterpret_cast<const uint8_t *>(&Value);
    Buffer.insert(Buffer.end(), Bytes, Bytes + sizeof(T));
  }

  template <typename T>
    requires std::is_trivially_copyable_v<T>
  void WriteField(std::vector<uint8_t> &Buffer, const std::vector<T> &Values)
  {
    WriteField(Buffer, static_cast<uint32_t>(Values.size()));
    for (const T &Value : Values)
    {
      WriteField(Buffer, Value);
    }
  }

  // Explicit-template-argument reads (ReadField<Type>(...)) can't be plain-overloaded the way
  // WriteField is - T isn't deduced from an argument, so both a POD and a vector<T> overload
  // would just get T substituted directly and only one can be right. A class template can be
  // partially specialized on std::vector<T>'s shape instead, so it dispatches correctly.
  template <typename T>
  struct FieldReader
  {
    static_assert(std::is_trivially_copyable_v<T>,
                  "RPC field type must be trivially copyable, or add a WriteField/FieldReader overload for it");

    static T Read(const std::vector<uint8_t> &Buffer, size_t &Offset)
    {
      T Value{};
      if (Offset + sizeof(T) > Buffer.size())
      {
        ENGINE_LOG_ERROR("RPC: buffer underrun reading %zu bytes at offset %zu (buffer size %zu)",
                          sizeof(T), Offset, Buffer.size());
        return Value;
      }
      std::memcpy(&Value, Buffer.data() + Offset, sizeof(T));
      Offset += sizeof(T);
      return Value;
    }
  };

  template <typename T>
  struct FieldReader<std::vector<T>>
  {
    static std::vector<T> Read(const std::vector<uint8_t> &Buffer, size_t &Offset)
    {
      uint32_t Count = FieldReader<uint32_t>::Read(Buffer, Offset);
      std::vector<T> Result;
      Result.reserve(Count);
      for (uint32_t Index = 0; Index < Count; ++Index)
      {
        Result.push_back(FieldReader<T>::Read(Buffer, Offset));
      }
      return Result;
    }
  };

  template <typename T>
  T ReadField(const std::vector<uint8_t> &Buffer, size_t &Offset)
  {
    return FieldReader<T>::Read(Buffer, Offset);
  }

  // ---- Wire ids: a compile-time hash of the RPC's own name, not a manually-synced enum.

  constexpr size_t ConstexprStrLen(const char *Str)
  {
    size_t Length = 0;
    while (Str[Length] != '\0')
    {
      ++Length;
    }
    return Length;
  }

  constexpr uint32_t FNV1aHash(const char *Str, size_t Length)
  {
    uint32_t Hash = 2166136261u;
    for (size_t Index = 0; Index < Length; ++Index)
    {
      Hash ^= static_cast<uint8_t>(Str[Index]);
      Hash *= 16777619u;
    }
    return Hash;
  }

  constexpr uint16_t HashName(const char *Name)
  {
    uint32_t Full = FNV1aHash(Name, ConstexprStrLen(Name));
    return static_cast<uint16_t>((Full ^ (Full >> 16)) & 0xFFFFu);
  }

  // ---- Dispatch table. Implemented in RPC.cpp (ENGINE_API), not as a header-only inline
  // function-local static: Game.exe and Engine.dll are separate binaries, and an inline
  // function's function-local static is only shared within one binary image, not across a
  // DLL/EXE boundary.
  using HandlerFn = std::function<void(EngineContext &, uint32_t /*Sender*/, const std::vector<uint8_t> & /*Payload*/)>;

  ENGINE_API void RegisterHandler(uint16_t WireId, HandlerFn Fn);
  ENGINE_API void Dispatch(EngineContext &Context, uint32_t Sender, const std::vector<uint8_t> &Payload);

  // For an RPC_CLIENT that's *always* broadcast unconditionally with no exceptions, prefer
  // RPC_MULTICAST instead - it generates a plain callable with no loop visible at the call site
  // at all. Use this one for the case RPC_MULTICAST can't cover: an RPC_CLIENT that's also
  // sometimes sent to one specific connection (so it can't become RPC_MULTICAST itself), or that
  // needs CallAllClientsExcept below - same call shape as calling the RPC_CLIENT function
  // directly, just naming every connection instead of one.
  template <typename Func, typename... Args>
    requires std::is_invocable_v<Func, EngineContext &, uint32_t, Args...>
  void CallAllClients(EngineContext &Context, Func &&RpcFn, Args &&...RpcArgs)
  {
    for (uint32_t ConnectionId : Context.Network.GetConnectedIds())
    {
      RpcFn(Context, ConnectionId, RpcArgs...);
    }
  }

  // Same, but skips one connection - e.g. telling existing clients about a newcomer, without
  // re-telling the newcomer about itself. ExcludedConnectionId is an ordinary caller-supplied
  // value, not derived from anything - there's no "Sender" to exclude automatically here
  // (RPC_CLIENT has none), so this can't be folded into RPC_MULTICAST or RPC_CLIENT itself; who
  // gets excluded varies per call site, not per RPC declaration.
  template <typename Func, typename... Args>
    requires std::is_invocable_v<Func, EngineContext &, uint32_t, Args...>
  void CallAllClientsExcept(EngineContext &Context, uint32_t ExcludedConnectionId, Func &&RpcFn, Args &&...RpcArgs)
  {
    for (uint32_t ConnectionId : Context.Network.GetConnectedIds())
    {
      if (ConnectionId != ExcludedConnectionId)
      {
        RpcFn(Context, ConnectionId, RpcArgs...);
      }
    }
  }
}

// ============================================================================================
// RPC_SERVER / RPC_CLIENT - bounded preprocessor FOR_EACH over up to 6 (Type, Name) pairs.
// Not a general-purpose reflection system: field types with an internal top-level comma (e.g.
// std::map<uint32_t, float>) will break the (Type, Name) pair-splitting - wrap in an extra
// paren or a `using` alias if that's ever needed. 0-param RPCs aren't supported (every RPC in
// this project has at least one field); add __VA_OPT__ handling if that's ever needed.
// ============================================================================================

#define RPC_EXPAND(X) X

#define RPC_ARG_COUNT(...) RPC_EXPAND(RPC_ARG_COUNT_(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define RPC_ARG_COUNT_(_1, _2, _3, _4, _5, _6, N, ...) N

#define RPC_CONCAT_(A, B) A##B
#define RPC_CONCAT(A, B) RPC_CONCAT_(A, B)

// Plain-concatenated (no separator) - for statement lists (write/read).
#define RPC_FOR_EACH_1(What, X) What(X)
#define RPC_FOR_EACH_2(What, X, ...) What(X) RPC_EXPAND(RPC_FOR_EACH_1(What, __VA_ARGS__))
#define RPC_FOR_EACH_3(What, X, ...) What(X) RPC_EXPAND(RPC_FOR_EACH_2(What, __VA_ARGS__))
#define RPC_FOR_EACH_4(What, X, ...) What(X) RPC_EXPAND(RPC_FOR_EACH_3(What, __VA_ARGS__))
#define RPC_FOR_EACH_5(What, X, ...) What(X) RPC_EXPAND(RPC_FOR_EACH_4(What, __VA_ARGS__))
#define RPC_FOR_EACH_6(What, X, ...) What(X) RPC_EXPAND(RPC_FOR_EACH_5(What, __VA_ARGS__))
#define RPC_FOR_EACH(What, ...) RPC_EXPAND(RPC_CONCAT(RPC_FOR_EACH_, RPC_ARG_COUNT(__VA_ARGS__))(What, __VA_ARGS__))

// Comma-joined - for parameter/argument lists.
#define RPC_FOR_EACH_COMMA_1(What, X) What(X)
#define RPC_FOR_EACH_COMMA_2(What, X, ...) What(X), RPC_EXPAND(RPC_FOR_EACH_COMMA_1(What, __VA_ARGS__))
#define RPC_FOR_EACH_COMMA_3(What, X, ...) What(X), RPC_EXPAND(RPC_FOR_EACH_COMMA_2(What, __VA_ARGS__))
#define RPC_FOR_EACH_COMMA_4(What, X, ...) What(X), RPC_EXPAND(RPC_FOR_EACH_COMMA_3(What, __VA_ARGS__))
#define RPC_FOR_EACH_COMMA_5(What, X, ...) What(X), RPC_EXPAND(RPC_FOR_EACH_COMMA_4(What, __VA_ARGS__))
#define RPC_FOR_EACH_COMMA_6(What, X, ...) What(X), RPC_EXPAND(RPC_FOR_EACH_COMMA_5(What, __VA_ARGS__))
#define RPC_FOR_EACH_COMMA(What, ...) RPC_EXPAND(RPC_CONCAT(RPC_FOR_EACH_COMMA_, RPC_ARG_COUNT(__VA_ARGS__))(What, __VA_ARGS__))

// (Type, Name) pair unwrapping.
#define RPC_PAIR_NAME(Pair) RPC_PAIR_NAME_ Pair
#define RPC_PAIR_NAME_(Type, Name) Name
#define RPC_PARAM_DECL(Pair) RPC_PARAM_DECL_ Pair
#define RPC_PARAM_DECL_(Type, Name) Type Name
#define RPC_WRITE_STMT(Pair) RPC_WRITE_STMT_ Pair
#define RPC_WRITE_STMT_(Type, Name) Engine::RPC::WriteField(Buffer__, Name);
#define RPC_READ_STMT(Pair) RPC_READ_STMT_ Pair
#define RPC_READ_STMT_(Type, Name) Type Name = Engine::RPC::ReadField<Type>(Payload__, Offset__);

#define RPC_WIRE_ID(Name) Engine::RPC::HashName(#Name)

// Declaration-only forms, for a header: lets a .cpp elsewhere call Name(...) (e.g. Game.cpp
// calling into a dedicated NetworkRPCs.cpp) without redefining it. Field list must match the
// matching RPC_SERVER/RPC_CLIENT definition exactly.
#define RPC_DECLARE_SERVER(Name, ...) \
  void Name(Engine::EngineContext &Context, RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));

#define RPC_DECLARE_CLIENT(Name, ...)                                       \
  void Name(Engine::EngineContext &Context, uint32_t TargetConnectionId__,  \
            RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));

// Client -> host. Name(Context, Fields...) is what you call: host runs the body directly (Sender
// == 0, meaning "the host itself" - see NetworkManager); a connected client sends it to the host
// instead; offline, it's a no-op. Sender in the body is always a transport-verified connection
// id, never client-supplied - a plain uint32_t, not the entity's NetworkId (see
// NetworkReplication.h for that split) - so it can be trusted for authority checks
// (FindEntityByOwner/HasAuthorityOverEntity) but never used as if it were a NetworkId directly.
#define RPC_SERVER(Name, ...)                                                                     \
  void Name##_Implementation(Engine::EngineContext &Context, uint32_t Sender,                      \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));                     \
  void Name(Engine::EngineContext &Context, RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))        \
  {                                                                                                 \
    if (Context.Network.IsHost())                                                                  \
    {                                                                                               \
      Name##_Implementation(Context, 0,                                                             \
                             RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__));                        \
    }                                                                                               \
    else if (Context.Network.GetHostConnectionId() != 0)                                            \
    {                                                                                               \
      std::vector<uint8_t> Buffer__;                                                                \
      Engine::RPC::WriteField(Buffer__, RPC_WIRE_ID(Name));                                         \
      RPC_FOR_EACH(RPC_WRITE_STMT, __VA_ARGS__)                                                     \
      Context.Network.SendReliable(Context.Network.GetHostConnectionId(), Buffer__.data(),          \
                                    static_cast<uint32_t>(Buffer__.size()));                        \
    }                                                                                                \
  }                                                                                                  \
  namespace                                                                                          \
  {                                                                                                   \
    struct Name##_Registrar                                                                          \
    {                                                                                                 \
      Name##_Registrar()                                                                             \
      {                                                                                               \
        Engine::RPC::RegisterHandler(                                                                \
            RPC_WIRE_ID(Name),                                                                       \
            [](Engine::EngineContext &Context, uint32_t Sender, const std::vector<uint8_t> &Payload__) \
            {                                                                                        \
              size_t Offset__ = sizeof(uint16_t);                                                    \
              RPC_FOR_EACH(RPC_READ_STMT, __VA_ARGS__)                                               \
              Name##_Implementation(Context, Sender, RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__)); \
            });                                                                                      \
      }                                                                                               \
    } Name##_RegistrarInstance;                                                                       \
  }                                                                                                    \
  void Name##_Implementation(Engine::EngineContext &Context, uint32_t Sender,                        \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))

#define RPC_DECLARE_SERVER_OWNED(Name, TargetPair, ...)                            \
  void Name(Engine::EngineContext &Context, RPC_PARAM_DECL(TargetPair),            \
            RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));

// Mechanically identical to RPC_DECLARE_SERVER - exists so a header can say, as plainly as the
// definition does, that this RPC's target field is deliberately NOT ownership-checked.
#define RPC_DECLARE_SERVER_UNCHECKED(Name, ...) RPC_DECLARE_SERVER(Name, __VA_ARGS__)

// Like RPC_SERVER, but TargetPair (always (uint32_t, SomeName)) is checked automatically - Sender
// must own the entity it names, or the call is rejected before your body ever runs (both when the
// host calls this locally and when a real message is dispatched - see RPC_SERVER's own comment
// for what Sender means in each case). Use this whenever a client is naming one of ITS OWN
// entities by id; use plain RPC_SERVER when there's no target field at all (Sender always means
// "my own entity" via FindEntityByOwner, nothing to check); use RPC_SERVER_UNCHECKED when the
// target is deliberately someone else's entity (attacking, trading, picking up their dropped
// item) - ownership is the wrong gate for those, a different, RPC-specific rule applies instead.
//
// Known limitation: requires TargetPair plus at least one more field, same as RPC_SERVER's
// existing "0-param RPCs aren't supported" limit - a target-only RPC would need RPC_FOR_EACH/
// RPC_ARG_COUNT extended with __VA_OPT__ for zero-to-N fields, not built here since nothing needs
// it yet.
#define RPC_SERVER_OWNED(Name, TargetPair, ...)                                                    \
  void Name##_Implementation(Engine::EngineContext &Context, uint32_t Sender,                       \
                              RPC_PARAM_DECL(TargetPair),                                            \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));                      \
  void Name(Engine::EngineContext &Context, RPC_PARAM_DECL(TargetPair),                              \
            RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))                                         \
  {                                                                                                  \
    if (Context.Network.IsHost())                                                                   \
    {                                                                                                \
      if (!Engine::HasAuthorityOverEntity(Context, 0, RPC_PAIR_NAME(TargetPair)))                    \
      {                                                                                              \
        return;                                                                                      \
      }                                                                                              \
      Name##_Implementation(Context, 0, RPC_PAIR_NAME(TargetPair),                                   \
                             RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__));                         \
    }                                                                                                 \
    else if (Context.Network.GetHostConnectionId() != 0)                                             \
    {                                                                                                 \
      std::vector<uint8_t> Buffer__;                                                                 \
      Engine::RPC::WriteField(Buffer__, RPC_WIRE_ID(Name));                                          \
      Engine::RPC::WriteField(Buffer__, RPC_PAIR_NAME(TargetPair));                                  \
      RPC_FOR_EACH(RPC_WRITE_STMT, __VA_ARGS__)                                                      \
      Context.Network.SendReliable(Context.Network.GetHostConnectionId(), Buffer__.data(),           \
                                    static_cast<uint32_t>(Buffer__.size()));                         \
    }                                                                                                  \
  }                                                                                                    \
  namespace                                                                                            \
  {                                                                                                     \
    struct Name##_Registrar                                                                            \
    {                                                                                                   \
      Name##_Registrar()                                                                               \
      {                                                                                                 \
        Engine::RPC::RegisterHandler(                                                                  \
            RPC_WIRE_ID(Name),                                                                         \
            [](Engine::EngineContext &Context, uint32_t Sender, const std::vector<uint8_t> &Payload__) \
            {                                                                                          \
              size_t Offset__ = sizeof(uint16_t);                                                      \
              RPC_READ_STMT(TargetPair)                                                                \
              if (!Engine::HasAuthorityOverEntity(Context, Sender, RPC_PAIR_NAME(TargetPair)))         \
              {                                                                                        \
                return;                                                                                \
              }                                                                                        \
              RPC_FOR_EACH(RPC_READ_STMT, __VA_ARGS__)                                                 \
              Name##_Implementation(Context, Sender, RPC_PAIR_NAME(TargetPair),                        \
                                     RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__));                   \
            });                                                                                        \
      }                                                                                                 \
    } Name##_RegistrarInstance;                                                                         \
  }                                                                                                      \
  void Name##_Implementation(Engine::EngineContext &Context, uint32_t Sender,                          \
                              RPC_PARAM_DECL(TargetPair),                                               \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))

// Mechanically identical to RPC_SERVER - exists so declaring one is a visible, grep-able choice
// ("this RPC's target is deliberately not ownership-checked") rather than a silent omission. Your
// body must implement whatever the actual validity rule is (range, alive target, not-same-team,
// whatever it is) and say so in a comment - nothing here does it for you.
#define RPC_SERVER_UNCHECKED(Name, ...) RPC_SERVER(Name, __VA_ARGS__)

// Host -> one specific client. Name(Context, TargetConnectionId, Fields...) always sends - only
// ever called by host code. Use Engine::RPC::CallAllClients to fan out to every connection.
#define RPC_CLIENT(Name, ...)                                                                     \
  void Name##_Implementation(Engine::EngineContext &Context,                                      \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));                    \
  void Name(Engine::EngineContext &Context, uint32_t TargetConnectionId__,                         \
            RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))                                       \
  {                                                                                                \
    std::vector<uint8_t> Buffer__;                                                                 \
    Engine::RPC::WriteField(Buffer__, RPC_WIRE_ID(Name));                                          \
    RPC_FOR_EACH(RPC_WRITE_STMT, __VA_ARGS__)                                                      \
    Context.Network.SendReliable(TargetConnectionId__, Buffer__.data(),                            \
                                  static_cast<uint32_t>(Buffer__.size()));                         \
  }                                                                                                 \
  namespace                                                                                         \
  {                                                                                                  \
    struct Name##_Registrar                                                                         \
    {                                                                                                \
      Name##_Registrar()                                                                            \
      {                                                                                              \
        Engine::RPC::RegisterHandler(                                                               \
            RPC_WIRE_ID(Name),                                                                      \
            [](Engine::EngineContext &Context, uint32_t /*Sender*/, const std::vector<uint8_t> &Payload__) \
            {                                                                                       \
              size_t Offset__ = sizeof(uint16_t);                                                   \
              RPC_FOR_EACH(RPC_READ_STMT, __VA_ARGS__)                                              \
              Name##_Implementation(Context, RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__));       \
            });                                                                                     \
      }                                                                                              \
    } Name##_RegistrarInstance;                                                                      \
  }                                                                                                   \
  void Name##_Implementation(Engine::EngineContext &Context,                                         \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))

// Mechanically identical to RPC_DECLARE_SERVER (no target/sender parameter either way) - kept as
// its own name for symmetry with RPC_DECLARE_CLIENT/RPC_DECLARE_SERVER_OWNED.
#define RPC_DECLARE_MULTICAST(Name, ...) RPC_DECLARE_SERVER(Name, __VA_ARGS__)

// Host -> everyone, including the host itself (run locally, not sent - see NetworkReplication.h/
// Unreal's own NetMulticast: a listen-server host sees its own effect because the function just
// ran locally, not because a message looped back to it). Name(Context, Fields...) is what you
// call - only ever from host code, matching RPC_CLIENT's own convention - and it's the whole
// call, no loop/CallAllClients needed at the call site. Only use this for an RPC that's *always*
// broadcast to everyone; if the same message is sometimes also sent to one specific connection
// (e.g. catching a newcomer up on existing state), that one needs RPC_CLIENT + a manual
// Engine::RPC::CallAllClients loop instead - Multicast can't target one connection at all.
#define RPC_MULTICAST(Name, ...)                                                                   \
  void Name##_Implementation(Engine::EngineContext &Context,                                       \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__));                     \
  void Name(Engine::EngineContext &Context, RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))        \
  {                                                                                                 \
    Name##_Implementation(Context, RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__));                 \
    std::vector<uint8_t> Buffer__;                                                                  \
    Engine::RPC::WriteField(Buffer__, RPC_WIRE_ID(Name));                                           \
    RPC_FOR_EACH(RPC_WRITE_STMT, __VA_ARGS__)                                                       \
    for (uint32_t ConnectionId__ : Context.Network.GetConnectedIds())                               \
    {                                                                                               \
      Context.Network.SendReliable(ConnectionId__, Buffer__.data(),                                 \
                                    static_cast<uint32_t>(Buffer__.size()));                        \
    }                                                                                                \
  }                                                                                                   \
  namespace                                                                                           \
  {                                                                                                    \
    struct Name##_Registrar                                                                           \
    {                                                                                                  \
      Name##_Registrar()                                                                              \
      {                                                                                                \
        Engine::RPC::RegisterHandler(                                                                 \
            RPC_WIRE_ID(Name),                                                                        \
            [](Engine::EngineContext &Context, uint32_t /*Sender*/, const std::vector<uint8_t> &Payload__) \
            {                                                                                         \
              size_t Offset__ = sizeof(uint16_t);                                                     \
              RPC_FOR_EACH(RPC_READ_STMT, __VA_ARGS__)                                                \
              Name##_Implementation(Context, RPC_FOR_EACH_COMMA(RPC_PAIR_NAME, __VA_ARGS__));         \
            });                                                                                       \
      }                                                                                                \
    } Name##_RegistrarInstance;                                                                        \
  }                                                                                                      \
  void Name##_Implementation(Engine::EngineContext &Context,                                            \
                              RPC_FOR_EACH_COMMA(RPC_PARAM_DECL, __VA_ARGS__))
