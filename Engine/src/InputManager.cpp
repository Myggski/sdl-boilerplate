
#include "InputManager.h"
#include "sdl/SDLEventDispatcher.h"
#include "Log.h"
#include <SDL3/SDL_keyboard.h>

namespace Engine
{
  InputData::InputData(SDL_Scancode Scancode) : Scancode(Scancode) {}

  InputManager::InputManager(SDLEventDispatcher &Dispatcher, SDL_Window *Window)
  {
    EventHandlers.reserve(8);

    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_KEY_DOWN, [this](const SDL_Event &Event)
                               { OnKeyPressed(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_KEY_UP, [this](const SDL_Event &Event)
                               { OnKeyReleased(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_TEXT_INPUT, [this](const SDL_Event &Event)
                               { OnTextInput(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_MOUSE_MOTION, [this](const SDL_Event &Event)
                               { OnMouseMotion(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_MOUSE_BUTTON_DOWN, [this](const SDL_Event &Event)
                               { OnMouseButtonDown(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_MOUSE_BUTTON_UP, [this](const SDL_Event &Event)
                               { OnMouseButtonUp(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_GAMEPAD_ADDED, [this](const SDL_Event &Event)
                               { OnGamepadDeviceAdded(Event); });
    EventHandlers.emplace_back(Dispatcher, SDL_EVENT_GAMEPAD_REMOVED, [this](const SDL_Event &Event)
                               { OnGamepadDeviceRemoved(Event); });

    // Unconditional: this is a desktop-only engine with no IME/on-screen-keyboard concerns that
    // would motivate gating text input to only-while-focused. SDL3 scopes text input to a
    // specific window (multi-window aware), unlike SDL2's global SDL_StartTextInput().
    SDL_StartTextInput(Window);
  }

  InputManager::~InputManager()
  {
    for (auto &[JoystickId, Controller] : Gamepads)
    {
      SDL_CloseGamepad(Controller);
    }
  }

  void InputManager::LateUpdate()
  {
    PreviousMouseButtons = MouseButtons;
    TextInputThisFrame.clear();
    KeysPressedThisFrame.clear();
  }

  void InputManager::OnKeyPressed(SDL_Event Event)
  {
    // Every SDL_EVENT_KEY_DOWN lands here, OS auto-repeats included, so this unconditionally
    // captures every scancode pressed this frame, repeats and all.
    KeysPressedThisFrame.push_back(Event.key.scancode);

    if (IsKeyReleased(Event.key.scancode))
    {
      Keys[Event.key.scancode] = InputData(Event.key.scancode);
    }
    else
    {
      Keys.at(Event.key.scancode).NumberOfRepeats += 1;
    }
  }

  void InputManager::OnKeyReleased(SDL_Event Event)
  {
    Keys.erase(Event.key.scancode);
  }

  void InputManager::OnTextInput(SDL_Event Event)
  {
    TextInputThisFrame += Event.text.text;
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
    // Event.gdevice.which is a transient device index here, not a stable id; opening the
    // gamepad is also required to receive any further events for it.
    SDL_Gamepad *Controller = SDL_OpenGamepad(Event.gdevice.which);
    if (!Controller)
    {
      ENGINE_LOG_ERROR("Failed to open gamepad %d: %s", Event.gdevice.which, SDL_GetError());
      return;
    }

    SDL_JoystickID JoystickId = SDL_GetJoystickID(SDL_GetGamepadJoystick(Controller));
    Gamepads[JoystickId] = Controller;

    GamepadConnected.Broadcast(JoystickId);
  }

  void InputManager::OnGamepadDeviceRemoved(SDL_Event Event)
  {
    // Unlike ADDED, Event.gdevice.which here already is the stable SDL_JoystickID.
    SDL_JoystickID JoystickId = Event.gdevice.which;

    auto Iterator = Gamepads.find(JoystickId);
    if (Iterator != Gamepads.end())
    {
      SDL_CloseGamepad(Iterator->second);
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
