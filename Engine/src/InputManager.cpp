
#include "InputManager.h"
#include "SDL/SDLEventDispatcher.h"

namespace Engine
{
  std::unique_ptr<InputManager> InputManager::Instance = nullptr;

  InputData::InputData(SDL_Scancode Scancode) : Scancode(Scancode) {}

  InputManager::InputManager()
  {
    // Subscribe to SDL events
    EventHandlers.reserve(5);
  }

  InputManager &InputManager::GetInstance()
  {
    if (!Instance)
    {
      Instance.reset(new InputManager());
    }
    return *Instance;
  }

  void InputManager::Initialize(SDLEventDispatcher &Dispatcher)
  {
    EventHandlers.emplace_back(Dispatcher, SDL_KEYDOWN, [](const SDL_Event &Event)
                               { GetInstance().OnKeyPressed(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_KEYUP, [](const SDL_Event &Event)
                               { GetInstance().OnKeyReleased(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_MOUSEMOTION, [](const SDL_Event &Event)
                               { GetInstance().OnMouseMotion(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_MOUSEBUTTONDOWN, [](const SDL_Event &Event)
                               { GetInstance().OnMouseButtonDown(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_MOUSEBUTTONUP, [](const SDL_Event &Event)
                               { GetInstance().OnMouseButtonUp(Event); });
  }

  void InputManager::Clear()
  {
    EventHandlers.clear();
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