#pragma once

#include "Core.h"
#include "PrecompiledHeader.h"
#include "sdl/SDLEventHandler.h"
#include <SDL_events.h>
#include <SDL_scancode.h>

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
    static InputManager &GetInstance(); // Returns the singleton instance

    void Initialize(SDLEventDispatcher &Dispatcher);
    void Clear();

    bool IsKeyPressed(const SDL_Scancode Scancode) const;
    bool IsKeyReleased(const SDL_Scancode Scancode) const;
    bool IsKeyHeld(const SDL_Scancode Scancode) const;
    bool IsMouseButtonPressed(Uint8 button) const;
    bool IsMouseButtonReleased(Uint8 button) const;
    inline int GetMouseX() const { return MouseX; }
    inline int GetMouseY() const { return MouseY; }

  private:
    InputManager(); // Private constructor for singleton

    void OnKeyPressed(SDL_Event Event);
    void OnKeyReleased(SDL_Event Event);
    void OnMouseMotion(SDL_Event Event);
    void OnMouseButtonDown(SDL_Event Event);
    void OnMouseButtonUp(SDL_Event Event);

  private:
    static std::unique_ptr<InputManager> Instance; // Singleton instance

    std::vector<SDLEventHandler> EventHandlers;

    std::unordered_map<SDL_Scancode, InputData> Keys;
    std::unordered_map<Uint8, bool> MouseButtons;         // Current button states
    std::unordered_map<Uint8, bool> PreviousMouseButtons; // Previous button states

    int MouseX{0};
    int MouseY{0};

    int KeyPressedEventId{-1};
    int KeyReleaseEventId{-1};
    int MousePressedEventId{-1};
    int MouseReleasedEventId{-1};
    int MouseMotionEventId{-1};

    // Game events
    GameEvent<SDL_Event>
        OnSDLEvent;
  };
}
