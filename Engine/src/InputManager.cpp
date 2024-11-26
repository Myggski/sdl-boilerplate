
#include "InputManager.h"
#include "EventManager.h"

namespace Engine
{
  std::unique_ptr<InputManager> InputManager::Instance{nullptr};

  InputData::InputData(SDL_Scancode Scancode) : Scancode(Scancode) {}

  InputManager::InputManager()
  {
    // Subscribe to SDL events
    Engine::EventManager &EventManager{EventManager::EventManager::GetInstance()};
    KeyPressedEventId = EventManager.RegisterEventListener(SDL_KEYDOWN, [](const SDL_Event &Event)
                                                           { GetInstance().OnKeyPressed(Event); });
    KeyReleaseEventId = EventManager.RegisterEventListener(SDL_KEYUP, [](const SDL_Event &Event)
                                                           { GetInstance().OnKeyReleased(Event); });
    MouseMotionEventId = EventManager.RegisterEventListener(SDL_MOUSEMOTION, [](const SDL_Event &Event)
                                                            { GetInstance().OnMouseMotion(Event); });
    MousePressedEventId = EventManager.RegisterEventListener(SDL_MOUSEBUTTONDOWN, [](const SDL_Event &Event)
                                                             { GetInstance().OnMouseButtonDown(Event); });
    MouseReleasedEventId = EventManager.RegisterEventListener(SDL_MOUSEBUTTONUP, [](const SDL_Event &Event)
                                                              { GetInstance().OnMouseButtonUp(Event); });
  }

  InputManager::~InputManager()
  {
    Engine::EventManager &EventManager{EventManager::EventManager::GetInstance()};
    EventManager.RemoveEventListener(SDL_KEYDOWN, KeyPressedEventId);
    EventManager.RemoveEventListener(SDL_KEYUP, KeyReleaseEventId);
    EventManager.RemoveEventListener(SDL_MOUSEMOTION, MouseMotionEventId);
    EventManager.RemoveEventListener(SDL_MOUSEBUTTONDOWN, MousePressedEventId);
    EventManager.RemoveEventListener(SDL_MOUSEBUTTONUP, MouseReleasedEventId);
  }

  InputManager &InputManager::GetInstance()
  {
    if (!Instance)
    {
      Instance.reset(new InputManager());
    }
    return *Instance;
  }

  GameEvent<SDL_Event> &InputManager::GetSDLEvent()
  {
    return OnSDLEvent;
  }

  void InputManager::OnKeyPressed(SDL_Event Event)
  {
    if (IsKeyReleased(Event.key.keysym.scancode))
    {
      Keys[Event.key.keysym.scancode] = InputData(Event.key.keysym.scancode);
    }
    else
    {
      Keys.at(Event.key.keysym.scancode).NumberOfRepeats += 1;
    }
  }

  void InputManager::OnKeyReleased(SDL_Event Event)
  {
    Keys.erase(Event.key.keysym.scancode);
  }

  void InputManager::OnMouseMotion(SDL_Event Event)
  {
    MouseX = Event.motion.x;
    MouseY = Event.motion.y;

    // Store or process the mouse position
    // For example, you might store it in a member variable
  }

  /// @brief
  /// @param Event
  void InputManager::OnMouseButtonDown(SDL_Event Event)
  {
    MouseButtons[Event.button.button] = true; // Mark the button as pressed
  }

  void InputManager::OnMouseButtonUp(SDL_Event Event)
  {
    MouseButtons[Event.button.button] = false; // Mark the button as released
  }

  bool InputManager::IsKeyPressed(const SDL_Scancode Scancode) const
  {
    return Keys.contains(Scancode);
  }

  bool InputManager::IsKeyReleased(const SDL_Scancode Scancode) const
  {
    return !IsKeyPressed(Scancode);
  }

  bool InputManager::IsKeyHeld(const SDL_Scancode Scancode) const
  {
    // Check if the key is pressed and has been held for a certain duration
    auto Iterator = Keys.find(Scancode);
    return Iterator != Keys.end() && Iterator->second.NumberOfRepeats >= 1;
  }

  bool InputManager::IsMouseButtonPressed(Uint8 button) const
  {
    auto Iterator = MouseButtons.find(button);
    return Iterator != MouseButtons.end() && Iterator->second;
  }

  bool InputManager::IsMouseButtonReleased(Uint8 button) const
  {
    return PreviousMouseButtons.at(button) && !MouseButtons.at(button);
  }
}