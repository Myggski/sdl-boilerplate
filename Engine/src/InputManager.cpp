
#include "InputManager.h"
#include "sdl/SDLEventDispatcher.h"
#include "Log.h"

namespace Engine
{
  InputData::InputData(SDL_Scancode Scancode) : Scancode(Scancode) {}

  InputManager::InputManager(SDLEventDispatcher &Dispatcher)
  {
    EventHandlers.reserve(7);

    EventHandlers.emplace_back(Dispatcher, SDL_KEYDOWN, [this](const SDL_Event &Event)
                               { OnKeyPressed(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_KEYUP, [this](const SDL_Event &Event)
                               { OnKeyReleased(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_MOUSEMOTION, [this](const SDL_Event &Event)
                               { OnMouseMotion(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_MOUSEBUTTONDOWN, [this](const SDL_Event &Event)
                               { OnMouseButtonDown(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_MOUSEBUTTONUP, [this](const SDL_Event &Event)
                               { OnMouseButtonUp(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_CONTROLLERDEVICEADDED, [this](const SDL_Event &Event)
                               { OnGamepadDeviceAdded(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_CONTROLLERDEVICEREMOVED, [this](const SDL_Event &Event)
                               { OnGamepadDeviceRemoved(Event); });
  }

  InputManager::~InputManager()
  {
    for (auto &[JoystickId, Controller] : Gamepads)
    {
      SDL_GameControllerClose(Controller);
    }
  }

  void InputManager::LateUpdate()
  {
    PreviousMouseButtons = MouseButtons;
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

  void InputManager::OnGamepadDeviceAdded(SDL_Event Event)
  {
    // Event.cdevice.which is a transient device index here, not a stable id; opening the
    // controller is also required to receive any further events for it.
    SDL_GameController *Controller = SDL_GameControllerOpen(Event.cdevice.which);
    if (!Controller)
    {
      ENGINE_LOG_ERROR("Failed to open gamepad %d: %s", Event.cdevice.which, SDL_GetError());
      return;
    }

    SDL_JoystickID JoystickId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(Controller));
    Gamepads[JoystickId] = Controller;

    GamepadConnected.Broadcast(JoystickId);
  }

  void InputManager::OnGamepadDeviceRemoved(SDL_Event Event)
  {
    // Unlike ADDED, Event.cdevice.which here already is the stable SDL_JoystickID.
    SDL_JoystickID JoystickId = Event.cdevice.which;

    auto Iterator = Gamepads.find(JoystickId);
    if (Iterator != Gamepads.end())
    {
      SDL_GameControllerClose(Iterator->second);
      Gamepads.erase(Iterator);
    }

    GamepadDisconnected.Broadcast(JoystickId);
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

  bool InputManager::IsMouseButtonPressed(Uint8 button, bool RespectUIClaim) const
  {
    if (RespectUIClaim && PointerClaimed)
    {
      return false;
    }

    auto Iterator = MouseButtons.find(button);
    return Iterator != MouseButtons.end() && Iterator->second;
  }

  bool InputManager::IsMouseButtonReleased(Uint8 button, bool RespectUIClaim) const
  {
    if (RespectUIClaim && PointerClaimed)
    {
      return false;
    }

    auto WasPressed = PreviousMouseButtons.find(button);
    auto IsPressed = MouseButtons.find(button);

    bool WasDown = WasPressed != PreviousMouseButtons.end() && WasPressed->second;
    bool IsDown = IsPressed != MouseButtons.end() && IsPressed->second;

    return WasDown && !IsDown;
  }

  bool InputManager::IsMouseButtonJustPressed(Uint8 button, bool RespectUIClaim) const
  {
    if (RespectUIClaim && PointerClaimed)
    {
      return false;
    }

    auto WasPressed = PreviousMouseButtons.find(button);
    auto IsPressed = MouseButtons.find(button);

    bool WasDown = WasPressed != PreviousMouseButtons.end() && WasPressed->second;
    bool IsDown = IsPressed != MouseButtons.end() && IsPressed->second;

    return IsDown && !WasDown;
  }
}
