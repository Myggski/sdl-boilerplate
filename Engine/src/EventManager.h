#pragma once

#include "Core.h"
#include "GameEvent.h"
#include <SDL_events.h>
#include <unordered_map>
#include <functional>

namespace Engine
{
  class ENGINE_API EventManager
  {
  public:
    static EventManager &GetInstance(); // Get singleton instance

    void PollEvents();                   // Poll and dispatch SDL events
    GameEvent<SDL_Event> &GetSDLEvent(); // Access the raw SDL event broadcaster

    int RegisterEventListener(uint32_t EventType, std::function<void(const SDL_Event &)> Callback);
    void RemoveEventListener(uint32_t EventType, uint32_t ListenerId);
    void SafelyRemovePendingEvents();

  private:
    EventManager() = default;

    static std::unique_ptr<EventManager> Instance; // Singleton instance

    std::unordered_map<uint32_t, GameEvent<SDL_Event>> EventMap; // Events by SDL type
    std::vector<std::pair<uint32_t, uint32_t>> PendingForRemoval;
    GameEvent<SDL_Event> RawSDLEvent;
  };
}
