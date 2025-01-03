#include "sdl/SDLEventDispatcher.h"
#include <iostream>
#include <vector>
#include <utility>

namespace Engine
{
  void SDLEventDispatcher::PollEvents()
  {
    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
      // Broadcast the raw event
      RawSDLEvent.Broadcast(Event);

      // If there's a specific event type listener, dispatch to it
      auto Iterator = EventMap.find(Event.type);
      if (Iterator != EventMap.end())
      {
        Iterator->second.Broadcast(Event);
      }
    }
  }

  GameEvent<SDL_Event> &SDLEventDispatcher::GetSDLEvent()
  {
    return RawSDLEvent;
  }

  int SDLEventDispatcher::RegisterEventListener(uint32_t EventType, std::function<void(const SDL_Event &)> Callback)
  {
    return EventMap[EventType].Add(Callback);
  }

  void SDLEventDispatcher::RemoveEventListener(uint32_t EventType, uint32_t ListenerId)
  {
    if (EventMap.contains(EventType))
    {
      EventMap[EventType].Remove(ListenerId);
    }
  }
}
