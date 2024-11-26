#include "EventManager.h"
#include <iostream>
#include <vector>
#include <utility>

namespace Engine
{
  std::unique_ptr<EventManager> EventManager::Instance{nullptr};

  EventManager &EventManager::GetInstance()
  {
    if (!Instance)
    {
      Instance.reset(new EventManager());
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

  int EventManager::RegisterEventListener(uint32_t EventType, std::function<void(const SDL_Event &)> Callback)
  {
    return EventMap[EventType].Add(Callback);
  }

  void EventManager::RemoveEventListener(uint32_t EventType, uint32_t ListenerId)
  {
    std::pair<uint32_t, uint32_t> PairToAdd(EventType, ListenerId);
    PendingForRemoval.push_back(PairToAdd);
  }

  void EventManager::SafelyRemovePendingEvents()
  {
    // Iterate by reference to avoid copying elements
    for (const std::pair<uint32_t, uint32_t> &RemoveData : PendingForRemoval)
    {
      if (EventMap.contains(RemoveData.first))
      {
        EventMap[RemoveData.first].Remove(RemoveData.second); // Remove the listener
      }
    }

    // Clear the list of pending removals
    PendingForRemoval.clear();
  }
}
