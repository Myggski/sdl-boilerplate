#pragma once

#include "Core.h"
#include "GameEvent.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace Engine
{
  // Plain aggregate, not ENGINE_API - same shape as UI::Rect/Size (Engine/src/ui/Widget.h).
  struct NetworkMessage
  {
    uint32_t ConnectionId = 0;
    std::vector<uint8_t> Payload;
  };

  // Plain aggregate, not ENGINE_API. Reports a connection completing or closing - there's no other
  // proactive signal for this (IsConnected() only lets you poll a specific id you already know
  // about). Deliberately says nothing about "host" or "client": that's game-layer vocabulary.
  struct ConnectionEvent
  {
    uint32_t ConnectionId = 0;
    bool Connected = false; // true = newly connected, false = newly closed
  };

  // A thin wrapper around GameNetworkingSockets. Transport only - no gameplay-state replication,
  // prediction/reconciliation, or session/lobby management; that's built on top of this in
  // Game.cpp. GameNetworkingSockets types never appear here (pImpl): only NetworkManager.cpp
  // includes them.
  class ENGINE_API NetworkManager
  {
  public:
    NetworkManager();
    ~NetworkManager();

    NetworkManager(const NetworkManager &) = delete;
    NetworkManager &operator=(const NetworkManager &) = delete;

    bool StartHost(uint16_t Port);

    // Returns a PENDING connection handle, not "connected" - GameNetworkingSockets' connection
    // handshake completes asynchronously (observed via Poll()), so check IsConnected() on a later
    // frame. 0 means the connection attempt itself couldn't even be created.
    uint32_t Connect(const char *Address, uint16_t Port);

    void Disconnect();

    // Pumps GameNetworkingSockets' internal callbacks (connection state changes, including
    // accepting inbound connections). Call once per real frame, before anything that reads
    // PollMessages()/IsConnected() this frame - see GameEngine.cpp's call site.
    void Poll();

    // Drains messages received since the last call.
    std::vector<NetworkMessage> PollMessages();

    // Drains connection-completed/closed events since the last call.
    std::vector<ConnectionEvent> PollConnectionEvents();

    void SendReliable(uint32_t ConnectionId, const void *Data, uint32_t Size);
    void SendUnreliable(uint32_t ConnectionId, const void *Data, uint32_t Size);

    bool IsConnected(uint32_t ConnectionId) const;

    bool IsHost() const;

    // For a client, the connection id Connect() returned. 0 if hosting or offline.
    uint32_t GetHostConnectionId() const;

    // Real remote connections only (excludes the host's own local id 0 - there's no transport
    // connection for "host talking to itself").
    std::vector<uint32_t> GetConnectedIds() const;

    // Fired once per newly-connected/closed connection (ConnectionId), on the fixed-step tick -
    // see GameEngine.cpp's call site for exactly when. NetworkManager only owns these, it doesn't
    // decide when to fire them, same as it has no opinion on fixed-step timing itself.
    GameEvent<uint32_t> &OnClientConnected() { return ClientConnected; }
    GameEvent<uint32_t> &OnClientDisconnected() { return ClientDisconnected; }

  private:
    struct Impl;
    std::unique_ptr<Impl> ImplPtr;

    GameEvent<uint32_t> ClientConnected;
    GameEvent<uint32_t> ClientDisconnected;
  };
}
