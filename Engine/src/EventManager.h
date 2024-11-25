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
    static void CreateInstance();       // Create singleton instance
    static EventManager &GetInstance(); // Get singleton instance

    void PollEvents();                   // Poll and dispatch SDL events
    GameEvent<SDL_Event> &GetSDLEvent(); // Access the raw SDL event broadcaster

    int RegisterEventListener(Uint32 EventType, std::function<void(const SDL_Event &)> Callback);
    bool RemoveEventListener(Uint32 EventType, int ListenerId);

  private:
    EventManager() = default;

    static std::unique_ptr<EventManager> Instance; // Singleton instance

    std::unordered_map<Uint32, GameEvent<SDL_Event>> EventMap; // Events by SDL type
    GameEvent<SDL_Event> RawSDLEvent;
  };
}
