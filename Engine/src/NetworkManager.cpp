#include "NetworkManager.h"
#include "Log.h"

// GameNetworkingSockets headers stay confined to this one .cpp - this is the entire point of the
// pImpl in NetworkManager.h. Do NOT add these to Engine/PrecompiledHeader.h: it's PUBLIC on the
// Engine target, so Game.cpp shares it, and that would leak the whole GNS header surface into
// every Engine .cpp and into Game.cpp.
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <cstring>

namespace Engine
{
  struct NetworkManager::Impl
  {
    ISteamNetworkingSockets *Interface = nullptr;
    HSteamListenSocket ListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup PollGroup = k_HSteamNetPollGroup_Invalid;
    bool Available = false;

    struct ConnectionEntry
    {
      HSteamNetConnection Handle;
      bool Connected = false;
    };
    std::vector<ConnectionEntry> Connections; // small N (a handful of players): linear scan is fine

    bool IsHostFlag = false;
    uint32_t HostConnectionId = 0;

    // Drained by PollConnectionEvents(). Pushed to for every connection this process is party to,
    // host or client alike (this callback doesn't know which role the process has) - a client's
    // own events just go unconsumed for the session, which is fine (one connection, a handful of
    // transitions total), not a leak.
    std::vector<ConnectionEvent> PendingConnectionEvents;

    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *Info);

    // GameNetworkingSockets' callback registration is a raw C function pointer (no user-data
    // parameter), so it can't capture `this` - route through a static pointer to the one
    // NetworkManager this process has instead (matches Valve's own example_chat.cpp pattern).
    static Impl *ActiveInstance;
    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *Info);
  };

  NetworkManager::Impl *NetworkManager::Impl::ActiveInstance = nullptr;

  void NetworkManager::Impl::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *Info)
  {
    if (ActiveInstance)
    {
      ActiveInstance->OnConnectionStatusChanged(Info);
    }
  }

  void NetworkManager::Impl::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *Info)
  {
    switch (Info->m_info.m_eState)
    {
    case k_ESteamNetworkingConnectionState_Connecting:
      // Only an INBOUND connection (arriving on our own listen socket) needs an explicit accept;
      // an outgoing Connect() reaching this state is just its own normal in-progress transition,
      // already tracked by Connect() itself.
      if (Info->m_info.m_hListenSocket != k_HSteamListenSocket_Invalid)
      {
        ENGINE_LOG_INFO("Network: incoming connection %u, accepting", (uint32_t)Info->m_hConn);
        EResult Result = Interface->AcceptConnection(Info->m_hConn);
        if (Result != k_EResultOK)
        {
          ENGINE_LOG_ERROR("Network: AcceptConnection failed (%d)", (int)Result);
          Interface->CloseConnection(Info->m_hConn, 0, nullptr, false);
        }
        else
        {
          Interface->SetConnectionPollGroup(Info->m_hConn, PollGroup);
          Connections.push_back({Info->m_hConn, false});
        }
      }
      break;

    case k_ESteamNetworkingConnectionState_Connected:
      for (ConnectionEntry &Entry : Connections)
      {
        if (Entry.Handle == Info->m_hConn)
        {
          Entry.Connected = true;
          ENGINE_LOG_INFO("Network: connection %u established", (uint32_t)Info->m_hConn);
          PendingConnectionEvents.push_back({(uint32_t)Info->m_hConn, true});
          break;
        }
      }
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
      ENGINE_LOG_INFO("Network: connection %u closed (%s)", (uint32_t)Info->m_hConn, Info->m_info.m_szEndDebug);
      Interface->CloseConnection(Info->m_hConn, 0, nullptr, false);
      for (size_t Index = 0; Index < Connections.size(); ++Index)
      {
        if (Connections[Index].Handle == Info->m_hConn)
        {
          PendingConnectionEvents.push_back({(uint32_t)Info->m_hConn, false});
          Connections.erase(Connections.begin() + Index);
          break;
        }
      }
      break;

    default:
      break;
    }
  }

  NetworkManager::NetworkManager() : ImplPtr(std::make_unique<Impl>())
  {
    SteamNetworkingErrMsg ErrMsg;
    if (!GameNetworkingSockets_Init(nullptr, ErrMsg))
    {
      ENGINE_LOG_ERROR("Failed to initialize GameNetworkingSockets: %s", ErrMsg);
      return;
    }

    ImplPtr->Available = true;
    ImplPtr->Interface = SteamNetworkingSockets();
    ImplPtr->PollGroup = ImplPtr->Interface->CreatePollGroup();

    Impl::ActiveInstance = ImplPtr.get();
    SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(&Impl::SteamNetConnectionStatusChangedCallback);
  }

  NetworkManager::~NetworkManager()
  {
    if (!ImplPtr->Available)
    {
      return;
    }

    Disconnect();

    if (ImplPtr->PollGroup != k_HSteamNetPollGroup_Invalid)
    {
      ImplPtr->Interface->DestroyPollGroup(ImplPtr->PollGroup);
    }
    if (Impl::ActiveInstance == ImplPtr.get())
    {
      Impl::ActiveInstance = nullptr;
    }

    GameNetworkingSockets_Kill();
  }

  bool NetworkManager::StartHost(uint16_t Port)
  {
    if (!ImplPtr->Available)
    {
      return false;
    }

    SteamNetworkingIPAddr LocalAddr;
    LocalAddr.Clear();
    LocalAddr.m_port = Port;

    ImplPtr->ListenSocket = ImplPtr->Interface->CreateListenSocketIP(LocalAddr, 0, nullptr);
    if (ImplPtr->ListenSocket == k_HSteamListenSocket_Invalid)
    {
      ENGINE_LOG_ERROR("Network: failed to listen on port %u", Port);
      return false;
    }

    ENGINE_LOG_INFO("Network: listening on port %u", Port);
    ImplPtr->IsHostFlag = true;
    return true;
  }

  uint32_t NetworkManager::Connect(const char *Address, uint16_t Port)
  {
    if (!ImplPtr->Available)
    {
      return 0;
    }

    SteamNetworkingIPAddr RemoteAddr;
    RemoteAddr.Clear();
    if (!RemoteAddr.ParseString(Address))
    {
      ENGINE_LOG_ERROR("Network: failed to parse address '%s'", Address);
      return 0;
    }
    RemoteAddr.m_port = Port;

    HSteamNetConnection Connection = ImplPtr->Interface->ConnectByIPAddress(RemoteAddr, 0, nullptr);
    if (Connection == k_HSteamNetConnection_Invalid)
    {
      ENGINE_LOG_ERROR("Network: failed to create connection to %s:%u", Address, Port);
      return 0;
    }

    ImplPtr->Interface->SetConnectionPollGroup(Connection, ImplPtr->PollGroup);
    ImplPtr->Connections.push_back({Connection, false});

    ENGINE_LOG_INFO("Network: connecting to %s:%u (connection %u)...", Address, Port, (uint32_t)Connection);
    ImplPtr->HostConnectionId = (uint32_t)Connection;
    return (uint32_t)Connection;
  }

  void NetworkManager::Disconnect()
  {
    if (!ImplPtr->Available)
    {
      return;
    }

    for (const Impl::ConnectionEntry &Entry : ImplPtr->Connections)
    {
      ImplPtr->Interface->CloseConnection(Entry.Handle, 0, "Disconnecting", false);
    }
    ImplPtr->Connections.clear();

    if (ImplPtr->ListenSocket != k_HSteamListenSocket_Invalid)
    {
      ImplPtr->Interface->CloseListenSocket(ImplPtr->ListenSocket);
      ImplPtr->ListenSocket = k_HSteamListenSocket_Invalid;
    }
  }

  void NetworkManager::Poll()
  {
    if (!ImplPtr->Available)
    {
      return;
    }
    ImplPtr->Interface->RunCallbacks();
  }

  std::vector<NetworkMessage> NetworkManager::PollMessages()
  {
    std::vector<NetworkMessage> Result;
    if (!ImplPtr->Available || ImplPtr->PollGroup == k_HSteamNetPollGroup_Invalid)
    {
      return Result;
    }

    ISteamNetworkingMessage *IncomingMessages[32];
    int MessageCount = ImplPtr->Interface->ReceiveMessagesOnPollGroup(ImplPtr->PollGroup, IncomingMessages, 32);
    while (MessageCount > 0)
    {
      for (int Index = 0; Index < MessageCount; ++Index)
      {
        ISteamNetworkingMessage *Message = IncomingMessages[Index];
        const uint8_t *Bytes = static_cast<const uint8_t *>(Message->m_pData);

        NetworkMessage Out;
        Out.ConnectionId = (uint32_t)Message->m_conn;
        Out.Payload.assign(Bytes, Bytes + Message->m_cbSize);
        Result.push_back(std::move(Out));

        Message->Release(); // pooled buffer - must Release() after copying the payload out, or it leaks
      }
      MessageCount = ImplPtr->Interface->ReceiveMessagesOnPollGroup(ImplPtr->PollGroup, IncomingMessages, 32);
    }
    return Result;
  }

  std::vector<ConnectionEvent> NetworkManager::PollConnectionEvents()
  {
    std::vector<ConnectionEvent> Result;
    Result.swap(ImplPtr->PendingConnectionEvents);
    return Result;
  }

  void NetworkManager::SendReliable(uint32_t ConnectionId, const void *Data, uint32_t Size)
  {
    if (!ImplPtr->Available)
    {
      return;
    }
    ImplPtr->Interface->SendMessageToConnection((HSteamNetConnection)ConnectionId, Data, Size, k_nSteamNetworkingSend_Reliable, nullptr);
  }

  void NetworkManager::SendUnreliable(uint32_t ConnectionId, const void *Data, uint32_t Size)
  {
    if (!ImplPtr->Available)
    {
      return;
    }
    ImplPtr->Interface->SendMessageToConnection((HSteamNetConnection)ConnectionId, Data, Size, k_nSteamNetworkingSend_Unreliable, nullptr);
  }

  bool NetworkManager::IsConnected(uint32_t ConnectionId) const
  {
    for (const Impl::ConnectionEntry &Entry : ImplPtr->Connections)
    {
      if (Entry.Handle == (HSteamNetConnection)ConnectionId)
      {
        return Entry.Connected;
      }
    }
    return false;
  }

  bool NetworkManager::IsHost() const
  {
    return ImplPtr->IsHostFlag;
  }

  uint32_t NetworkManager::GetHostConnectionId() const
  {
    return ImplPtr->HostConnectionId;
  }

  std::vector<uint32_t> NetworkManager::GetConnectedIds() const
  {
    std::vector<uint32_t> Result;
    for (const Impl::ConnectionEntry &Entry : ImplPtr->Connections)
    {
      if (Entry.Connected)
      {
        Result.push_back((uint32_t)Entry.Handle);
      }
    }
    return Result;
  }
}
