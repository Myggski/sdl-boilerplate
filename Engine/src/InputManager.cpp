
#include "InputManager.h"

namespace Engine
{
  std::unique_ptr<InputManager> InputManager::Instance{nullptr};

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

  void InputManager::Pull()
  {
    SDL_Event Event;
    PreviousMouseButtons = MouseButtons;

    while (SDL_PollEvent(&Event))
    {
      switch (Event.type)
      {
      case SDL_KEYDOWN:
        Engine::InputManager::GetInstance().OnKeyPressed(Event);
        break;
      case SDL_KEYUP:
        Engine::InputManager::GetInstance().OnKeyReleased(Event);
        break;
      case SDL_MOUSEMOTION:
        Engine::InputManager::GetInstance().OnMouseMotion(Event);
        break;
      case SDL_MOUSEBUTTONDOWN:
        Engine::InputManager::GetInstance().OnMouseButtonDown(Event);
        break;
      case SDL_MOUSEBUTTONUP:
        Engine::InputManager::GetInstance().OnMouseButtonUp(Event);
        break;
      }

      OnSDLEvent.Broadcast(Event);
    }
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