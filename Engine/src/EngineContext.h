#pragma once

#include "Core.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Camera.h"
#include "sdl/SDLEventDispatcher.h"
#include "ecs/EntityManager.h"
#include "ui/Canvas.h"
#include "DebugOverlay.h"
#include "NetworkManager.h"

namespace Engine
{
  // Everything a Game callback needs, owned in one place and constructed once the window and
  // renderer exist. Member order matters: Assets/Input/Overlay are destroyed before
  // Renderer/Dispatcher, since their destructors need those still alive.
  struct ENGINE_API EngineContext
  {
    // DisplayScale is the fixed pixel-art upscale, not the camera's own runtime zoom.
    EngineContext(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth = 320, uint16_t ScreenHeight = 180, uint8_t DisplayScale = 6);

    EngineContext(const EngineContext &) = delete;
    EngineContext &operator=(const EngineContext &) = delete;

    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDLEventDispatcher Dispatcher;
    AssetManager Assets;
    InputManager Input;
    Camera MainCamera;
    EntityManager World;  // the ECS: create entities, add components. Movement/Animation/
                          // Collision are already registered (see EngineContext.cpp); register
                          // any of your own game-specific systems here too
    UI::Canvas UICanvas;  // the retained UI tree: menus, HUD, etc.; renders on top of the game
    DebugOverlay Overlay; // Dear ImGui; a no-op in Release builds, see DebugOverlay.h
    NetworkManager Network; // GameNetworkingSockets wrapper; Poll() once per real frame, see GameEngine.cpp
  };
}
