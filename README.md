# sdl-boilerplate

A personal 2D game engine + game project built on SDL2/C++23. Goals: get hands-on with CMake and
engine architecture, and end up with a reusable base for game jams.

Engine/Game split: `Engine/` is the reusable library (DLL), `Game/` is where you write a game.

## Build & run

Requires LLVM/Clang + Ninja on `PATH`, and Dear ImGui sources under `%CMAKE_PREFIX_PATH%/imgui`
(e.g. `C:\Code\sdks\imgui`), that's the only manual dependency. SDL2/SDL2_image/SDL2_ttf are
fetched and built automatically via `cmake/Dependencies.cmake`.

```
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Other presets: `x64-release`, `x64-debug-msvc` (MSVC, CMake-only for now, no VS Code debug wiring
yet). VS Code: "Build Debug/Release x64" tasks + matching lldb launch configs in `.vscode/`.

## Repo layout

```
Engine/
  Core.h, Engine.h, EntryPoint.h, GameEngine.h/.cpp   DLL export macros, umbrella include, main(),
                                                        fixed-timestep loop calling into GameEngineData
  assets/fonts/Roboto/    Engine-owned default UI font (Apache-2.0), copied next to the built
                          executable same as Game/assets/
  src/
    EngineContext.h/.cpp   Bundles Window/Renderer/Assets/Input/Camera/World/UICanvas/Overlay,
                            passed by reference through the Startup/Update/Draw/Shutdown callbacks
    AssetManager.h/.cpp    Texture + font cache, LoadDefaultFont() for the bundled Roboto font
    InputManager.h/.cpp    Keyboard/mouse/gamepad state, plus the UI pointer-claim flag
    Camera.h/.cpp          Screen-space zoom/scale for pixel art
    DebugOverlay.h/.cpp    Dear ImGui setup/frame bracketing, Debug builds only
    ecs/                   Entity/EntityManager/ComponentArray, GetComponentId<T>() for automatic
                            type-safe component ids
    ui/                    Retained-mode UI: Widget, BoxContainer, HorizontalBox/VerticalBox,
                            Panel, Text, Image, Button, Checkbox, Canvas (layout + hit-testing +
                            input claim)

Game/
  Game.h/.cpp        The file to write a game in: Startup/Update/Draw/Shutdown
  Bootstrap.cpp      Framework glue implementing Engine::CreateGameEngineData(); no need to open it
  assets/            Game content (images); the engine's own default font lives in Engine/assets/

cmake/Dependencies.cmake   FetchContent for SDL2/SDL2_image/SDL2_ttf
CMakePresets.json           x64-debug/x64-release/x64-debug-msvc
```

## Known gotchas

- An `ENGINE_API` class's inline `= default` constructor won't get its DLL symbol exported unless
  something inside Engine's own compiled sources calls it. Declare it in the header, define it
  (even `= default`) in the matching `.cpp`.
- Adding a new `.cpp` file needs a reconfigure (`cmake --preset ...`), not just a rebuild, the
  `file(GLOB ...)` source list won't pick it up otherwise.
- `AnimationSystem`/`SpriteComponent` exist but aren't wired into anything yet; `Game.cpp` draws
  its sprite directly in `Draw` instead.
- `x64-debug-msvc` only covers the CMake configure/build side, no VS Code launch config yet.
- The UI system has no `Dropdown`/`Scalar` widgets and no MVVM/data-binding layer yet.
