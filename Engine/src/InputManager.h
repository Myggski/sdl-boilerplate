#pragma once

#include "Core.h"
#include "PrecompiledHeader.h"
#include "sdl/SDLEventHandler.h"
#include <SDL_events.h>
#include <SDL_scancode.h>
#include <SDL_gamecontroller.h>

namespace Engine
{
  struct InputData
  {
  public:
    bool IsHeld() const { return NumberOfRepeats > 0; }
    InputData() = default;
    InputData(SDL_Scancode Scancode);

  public:
    size_t NumberOfRepeats{0};
    SDL_Scancode Scancode{SDL_SCANCODE_UNKNOWN};
  };

  class ENGINE_API InputManager
  {
  public:
    explicit InputManager(SDLEventDispatcher &Dispatcher);
    ~InputManager();

    InputManager(const InputManager &) = delete;
    InputManager &operator=(const InputManager &) = delete;

    // Snapshots the current input state as "previous" state. Call once per frame, after input
    // for the frame has been processed, so IsKeyReleased/IsMouseButtonReleased can compare
    // against it next frame.
    void LateUpdate();

    bool IsKeyPressed(const SDL_Scancode Scancode) const;
    bool IsKeyReleased(const SDL_Scancode Scancode) const;
    bool IsKeyHeld(const SDL_Scancode Scancode) const;

    // RespectUIClaim (default on) makes these return false while the UI has claimed the pointer
    // (see SetPointerClaimed below), so gameplay reading mouse input doesn't also react to a
    // click that landed on a widget. Pass false to bypass that and read the raw state regardless,
    // which the UI system itself needs to do when it's the one deciding whether to claim in the
    // first place.
    bool IsMouseButtonPressed(Uint8 button, bool RespectUIClaim = true) const;
    bool IsMouseButtonReleased(Uint8 button, bool RespectUIClaim = true) const;
    bool IsMouseButtonJustPressed(Uint8 button, bool RespectUIClaim = true) const;
    inline int GetMouseX() const { return MouseX; }
    inline int GetMouseY() const { return MouseY; }

    // Set once per frame by Engine::UI::Canvas after hit-testing, before gameplay reads input;
    // InputManager itself knows nothing about widgets or the UI tree, this is just a flag someone
    // else computes and it stores, the same way it stores raw SDL-reported mouse position.
    void SetPointerClaimed(bool Claimed) { PointerClaimed = Claimed; }
    bool IsPointerClaimed() const { return PointerClaimed; }

    // Fires with the SDL_JoystickID of the gamepad that was just connected/disconnected. That id
    // stays stable for the life of the connection, unlike the transient device index SDL's own
    // SDL_CONTROLLERDEVICEADDED event carries, so it's safe to use as a key to track a specific
    // gamepad across both events.
    GameEvent<SDL_JoystickID> &OnGamepadConnected() { return GamepadConnected; }
    GameEvent<SDL_JoystickID> &OnGamepadDisconnected() { return GamepadDisconnected; }

  private:
    void OnKeyPressed(SDL_Event Event);
    void OnKeyReleased(SDL_Event Event);
    void OnMouseMotion(SDL_Event Event);
    void OnMouseButtonDown(SDL_Event Event);
    void OnMouseButtonUp(SDL_Event Event);
    void OnGamepadDeviceAdded(SDL_Event Event);
    void OnGamepadDeviceRemoved(SDL_Event Event);

  private:
    std::vector<SDLEventHandler> EventHandlers;

    std::unordered_map<SDL_Scancode, InputData> Keys;
    std::unordered_map<Uint8, bool> MouseButtons;         // Current button states
    std::unordered_map<Uint8, bool> PreviousMouseButtons; // Previous button states

    int MouseX{0};
    int MouseY{0};

    bool PointerClaimed{false};

    // Gamepads currently open, keyed by the stable SDL_JoystickID, so they can be closed
    // correctly (individually on disconnect, or all of them on shutdown).
    std::unordered_map<SDL_JoystickID, SDL_GameController *> Gamepads;
    GameEvent<SDL_JoystickID> GamepadConnected;
    GameEvent<SDL_JoystickID> GamepadDisconnected;
  };
}
