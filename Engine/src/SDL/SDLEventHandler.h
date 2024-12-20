#pragma once

#include <functional>
#include <unordered_map>
#include <SDL_events.h>
#include "SDLEventDispatcher.h"

namespace Engine
{
  class ENGINE_API SDLEventHandler
  {
  public:
    SDLEventHandler(SDLEventDispatcher &Dispatcher, uint32_t EventType, std::function<void(const SDL_Event &)> Callback)
        : Dispatcher(Dispatcher), EventType(EventType), Callback(std::move(Callback))
    {
      ListenerId = Dispatcher.RegisterEventListener(EventType, this->Callback);
    }

    ~SDLEventHandler()
    {
      Dispatcher.RemoveEventListener(EventType, ListenerId);
    }

    // Copy constructor
    SDLEventHandler(const SDLEventHandler &other)
        : Dispatcher(other.Dispatcher), EventType(other.EventType), Callback(other.Callback)
    {
      // Register a new listener with the same EventType and Callback
      ListenerId = Dispatcher.RegisterEventListener(EventType, Callback);
    }

    // Copy assignment operator
    SDLEventHandler &operator=(const SDLEventHandler &other)
    {
      if (this != &other)
      {
        // First clean up the current state
        Dispatcher.RemoveEventListener(EventType, ListenerId);

        // Copy the state from the other object
        Dispatcher = other.Dispatcher; // Keeping this as a reference (unchanged)
        EventType = other.EventType;
        Callback = other.Callback;
        ListenerId = Dispatcher.RegisterEventListener(EventType, Callback);
      }
      return *this;
    }

    // Move constructor
    SDLEventHandler(SDLEventHandler &&other) noexcept
        : Dispatcher(other.Dispatcher), EventType(other.EventType), ListenerId(other.ListenerId), Callback(std::move(other.Callback))
    {
      // Reset the moved-from object to a valid state
      other.ListenerId = -1;
    }

    // Move assignment operator
    SDLEventHandler &operator=(SDLEventHandler &&other) noexcept
    {
      if (this != &other)
      {
        // First clean up the current state
        Dispatcher.RemoveEventListener(EventType, ListenerId);

        // Move the data from the other object
        Dispatcher = other.Dispatcher; // Dispatcher stays the same (reference)
        EventType = other.EventType;
        ListenerId = other.ListenerId;
        Callback = std::move(other.Callback);

        // Reset the moved-from object to a valid state
        other.ListenerId = -1;
      }
      return *this;
    }

  private:
    SDLEventDispatcher &Dispatcher;
    uint32_t EventType;
    int ListenerId;
    std::function<void(const SDL_Event &)> Callback; // Store the callback for copy/move operations
  };
}
