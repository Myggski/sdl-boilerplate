
#include "InputManager.h"
#include "EventManager.h"

namespace Engine
{
  std::unique_ptr<InputManager> InputManager::Instance{nullptr};

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

  InputData::InputData(SDL_Scancode Scancode) : Scancode(Scancode) {}

  void InputManager::CreateInstance()
  {
    if (Instance != nullptr)
    {
      throw std::logic_error("Instance of InputManager has already been created!");
    }

    Instance.reset(new InputManager()); // Use reset to create the instance
  }

  InputManager &InputManager::GetInstance()
  {
    if (Instance == nullptr)
    {
      throw std::logic_error("Instance of InputManager has not been created yet!");
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

  bool InputManager::IsKeyPressedOnce(const SDL_Scancode Scancode)
  {
    if (!IsKeyPressed(Scancode))
    {
      return false;
    }

    Keys.at(Scancode).NumberOfRepeats += 1;

    return IsKeyPressed(Scancode) && Keys.at(Scancode).NumberOfRepeats == 1;
  }

  bool InputManager::IsKeyReleased(const SDL_Scancode Scancode) const
  {
    return !IsKeyPressed(Scancode);
  }

  bool InputManager::IsKeyHeld(const SDL_Scancode Scancode) const
  {
    return Keys.contains(Scancode);
  }

  bool InputManager::IsMouseButtonPressed(Uint8 button) const
  {
    auto it = MouseButtons.find(button);
    return it != MouseButtons.end() && it->second;
  }

  bool InputManager::IsMouseButtonReleased(Uint8 button) const
  {
    return PreviousMouseButtons.at(button) && !MouseButtons.at(button);
  }
}