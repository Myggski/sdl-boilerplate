# sdl-boilerplate

A personal 2D game engine + game project (C++23). Goals: get hands-on with CMake and engine
architecture, and end up with a reusable base for game jams.

Engine/Game split: `Engine/` is the reusable library (DLL), `Game/` is where you write a game.

## Build & run

Requires LLVM/Clang + Ninja on `PATH`, and Dear ImGui sources under `%CMAKE_PREFIX_PATH%/imgui`
(e.g. `C:\Code\sdks\imgui`), the only manual dependency. SDL2/SDL2_image/SDL2_ttf are fetched and
built automatically via `cmake/Dependencies.cmake`.

```
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Other presets: `x64-release`, `x64-debug-msvc` (MSVC, CMake-only for now). VS Code: "Build
Debug/Release x64" tasks + `cppvsdbg` launch configs in `.vscode/` (not lldb: this toolchain's
PDB debug info needs the native Windows debugger to resolve reliably).

To package a build for distribution: `cmake --install build/x64-release` copies just the `.exe`,
the DLLs it needs, and `assets/` into `install/x64/`, ready to zip. `--prefix <dir>` to install
elsewhere.

## Repo layout

```
Engine/
  Core.h, Engine.h, EntryPoint.h, GameEngine.h/.cpp   Export macros, umbrella include, main(),
                                                        fixed-timestep game loop
  assets/fonts/Roboto/    Bundled default UI font
  src/
    EngineContext.h/.cpp   Window/Renderer/Assets/Input/Camera/World/UICanvas/Overlay, passed
                            through Startup/Update/Draw/Shutdown
    AssetManager.h/.cpp    Texture + font cache
    InputManager.h/.cpp    Keyboard/mouse/gamepad state
    Camera.h/.cpp          Pixel-art zoom/scale, DrawSprite
    DebugOverlay.h/.cpp    Dear ImGui setup (Debug builds only)
    math/                  Vector2D, Rect
    ecs/                   EntityManager + components (Transform, Velocity, Sprite, Animation)
                            + systems (Movement, Animation, Render)
    ui/                    Retained-mode UI: Widget, Canvas, Theme (colors/spacing)
      layout/                BoxContainer, HorizontalBox, VerticalBox
      input/                 Button, Checkbox, Scalar, Dropdown
      display/               Panel, Text, Image

Game/
  Game.h/.cpp        Write your game here: Startup/Update/Draw/Shutdown; the only files meant to
                      be edited to make a game
  framework/          Glue + settings, out of the way of Game.h/.cpp
    Bootstrap.cpp       Wires Game's callbacks into the engine; no need to open it
    CollisionSettings.h   Collision layers + CollisionMatrix, header-only (see Engine/src/ecs/CollisionMatrix.h)
  assets/            Game content

cmake/Dependencies.cmake   FetchContent for SDL2/SDL2_image/SDL2_ttf
CMakePresets.json           x64-debug/x64-release/x64-debug-msvc
```

## Design system

`Engine::UI::Theme` (`Engine/src/ui/Theme.h`) is the single source of truth for UI colors and
spacing, widgets and game code should pull from it rather than hardcoding values.

- **Colors**: `Engine::UI::Color`, never `SDL_Color`. The [Lospec "31" palette](https://lospec.com/palette-list/31)
  lives under `Theme::Palette`, used only through semantic roles (`PrimaryNormal/Hovered/Pressed`,
  `Success*`, `Danger*`, `Neutral*`, `Background`, `Surface`, `TextPrimary/Secondary`, `Accent`) so
  a palette swap means editing one file.
- **Spacing**: an 8px grid (`Theme::Spacing::XSmall` through `XXXLarge`) and a 48px minimum touch
  target (`Theme::MinTouchTarget`).

## Known gotchas

- An `ENGINE_API` class's inline `= default` constructor won't get its DLL symbol exported unless
  something inside Engine's own compiled sources calls it. Declare it in the header, define it
  (even `= default`) in the matching `.cpp`.
- Adding a new `.cpp` file needs a reconfigure (`cmake --preset ...`), not just a rebuild: the
  `file(GLOB ...)` source list won't pick it up otherwise. `Game/`'s own source list isn't a glob
  at all, so a new `Game/` `.cpp` also needs adding to `SOURCE_FILES` in `Game/CMakeLists.txt`.
- `Engine/EntryPoint.h` defines `main()` inline. Only `#include "Engine.h"` (the umbrella, which
  pulls it in) from exactly one `Game/` `.cpp`. A second one including it too duplicates `main()`
  at link time. Other `Game/` files needing engine types should include the specific header they
  need instead (e.g. `#include "src/ecs/CollisionMatrix.h"`), not the umbrella.
- `x64-debug-msvc` only covers the CMake configure/build side, no VS Code launch config yet.
- `Dropdown` has no popup/layering system to float on (`Canvas` doesn't have one yet), so its open
  option list can be drawn over by anything rendered after it in tree order. Rare in practice.
- The UI system has no MVVM/data-binding layer yet.
