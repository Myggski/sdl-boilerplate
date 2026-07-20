# sdl-boilerplate

A personal 2D game engine + game project (C++23). Goals: get hands-on with CMake and engine
architecture, and end up with a reusable base for game jams.

Engine/Game split: `Engine/` is the reusable library (DLL), `Game/` is where you write a game.

## Build & run

### First-time setup (once per machine)

1. Install [CMake](https://cmake.org/download/) 3.24+, LLVM/Clang, and
   [Ninja](https://github.com/ninja-build/ninja/releases), all on `PATH`
   (`choco install cmake llvm ninja` works too).
2. Install [vcpkg](https://github.com/microsoft/vcpkg):
   ```
   git clone https://github.com/microsoft/vcpkg C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   setx VCPKG_ROOT C:\vcpkg
   ```
   Open a new terminal afterward - `setx` doesn't affect ones already open. This is where all of
   this project's third-party dependencies come from (see `vcpkg.json`); nothing else to install.
3. Clone this repo and run one of the commands or scripts below.

The very first configure has to build every vcpkg dependency from source (nothing cached yet) -
expect 10-15 *minutes*, not seconds. Every configure after that reuses vcpkg's binary cache and is
fast (single-digit seconds).

### Everyday use

```
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Other presets: `x64-release`, `x64-debug-msvc` (MSVC, CMake-only for now - see Known gotchas).
VS Code: "Build Debug/Release x64" tasks + `cppvsdbg` launch configs in `.vscode/` (not lldb: this
toolchain's PDB debug info needs the native Windows debugger to resolve reliably).

If `sdl3`/`sdl3-image`/`sdl3-ttf` aren't found during configure, your vcpkg checkout predates
SDL3's addition to the vcpkg registry - `git pull` + re-run `bootstrap-vcpkg.bat` inside it.

## Helper scripts (`scripts/`)

- **`install.bat`** - builds Release, installs it, and zips the result to
  `install/sdl-boilerplate-x64.zip`: just the `.exe`, the DLLs it needs, and `assets/`, no
  `.pdb`/`.lib`/build artifacts. This is the easy path when you want a build to hand to someone
  else (e.g. to playtest multiplayer over a real network). Under the hood it's just
  `cmake --preset x64-release`, `cmake --build --preset x64-release`, then
  `cmake --install build/x64-release` (`--prefix <dir>` to install elsewhere instead) and a
  `Compress-Archive`.
- **`rebuild-all.bat`** - deletes and fully reconfigures+rebuilds both `x64-debug` and
  `x64-release` from scratch. Cheap enough to run casually (~10-15s each *once vcpkg's binary
  cache is warm* - see First-time setup above for why the very first run anywhere is much
  slower) whenever you want to confirm both configs still build cleanly, not just incrementally.
- **`host-or-join.bat`** - test multiplayer locally (needs `x64-debug` already built). First run
  becomes the host, every run after that joins it as another client - run it as many times as the
  number of players you want to test with.

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

CMakePresets.json           x64-debug/x64-release/x64-debug-msvc
vcpkg.json                  Third-party dependencies, see Build & run above
scripts/                     install.bat, rebuild-all.bat, host-or-join.bat - see Build & run above
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
- `x64-debug-msvc` only covers the CMake configure/build side, no VS Code launch config yet. It's
  also currently broken out of the box: SDL3_image's vendored `dav1d` (AV1 decoder) needs a NASM
  assembler under MSVC, and this repo doesn't install one. Install NASM and put it on `PATH` to
  unblock it; the default `x64-debug` (Clang) preset doesn't hit this.
- `Dropdown` has no popup/layering system to float on (`Canvas` doesn't have one yet), so its open
  option list can be drawn over by anything rendered after it in tree order. Rare in practice.
- The UI system has no MVVM/data-binding layer yet.
- `Game::Update()` runs inside `GameEngine`'s fixed-timestep loop, which can run zero, one, or
  several times per real (rendered) frame, so a discrete input check like
  `InputManager::IsMouseButtonJustPressed` can silently miss a click if its one-frame edge lands
  on a real frame the fixed step didn't run on. Don't read it from `Update()`; read it from
  `PreUpdate()` instead (called once per real frame, before the fixed-step loop), translate it into
  a command, and push it onto a queue for `Update()` to drain. A queue, not a single latched flag,
  since a second command arriving before the next `Update()` call would otherwise overwrite the
  first, and because this is also the shape networked input will take later (a remote peer's
  commands feeding the same queue). See `Game.cpp`'s `PendingMoveCommands` for a worked example.
