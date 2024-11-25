#include "EventManager.h"
#include <iostream>

namespace Engine
{
  std::unique_ptr<EventManager> EventManager::Instance{nullptr};

  void EventManager::CreateInstance()
  {
    if (Instance != nullptr)
    {
      throw std::logic_error("EventManager instance already exists!");
    }
    Instance.reset(new EventManager());
  }

  EventManager &EventManager::GetInstance()
  {
    if (Instance == nullptr)
    {
      throw std::logic_error("EventManager instance not created yet!");
    }
    return *Instance;
  }

  void EventManager::PollEvents()
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

  GameEvent<SDL_Event> &EventManager::GetSDLEvent()
  {
    return RawSDLEvent;
  }

  int EventManager::RegisterEventListener(Uint32 EventType, std::function<void(const SDL_Event &)> Callback)
  {
    return EventMap[EventType].Add(Callback);
  }

  bool EventManager::RemoveEventListener(Uint32 EventType, int ListenerId)
  {
    if (EventMap.contains(EventType))
    {
      EventMap[EventType].Remove(ListenerId);
      return true;
    }
    return false;
  }
}
