# sdl-boilerplate

A personal 2D game engine + game project built on SDL2, written in modern C++23. Two goals drive it: (1) get hands-on with CMake, compiler-agnostic builds, and engine architecture, and (2) end up with a reusable base for game jams and for visually demonstrating algorithms.

Goal: a small, simple-to-pick-up 2D engine, staying in C++ (for now, see "Future direction").

## Repo layout

```
Engine/                 Engine library (builds as a SHARED lib / DLL: sdl-boilerplate-engine)
  Core.h                ENGINE_API export/import macros, ENGINE_ASSERT macros, BIT()
  Engine.h              Umbrella include for consumers (Game.cpp includes this)
  EntryPoint.h           main(), calls Engine::CreateGameEngineData(), which Game/Bootstrap.cpp defines
  GameEngine.h/.cpp      Owns the SDL window/renderer, fixed-timestep game loop, calls into GameEngineData
                          callbacks (Startup/Update/Draw/Shutdown) supplied by the Game
  PrecompiledHeader.h/.cpp  PCH: <iostream>, <memory>, <functional>, <vector>, etc. + GameEvent.h
  src/
    EngineContext.h/.cpp   Bundles Window/Renderer/Dispatcher/Assets/Input/MainCamera/World/UICanvas/
                          Overlay into one owned object, constructed once the window and renderer
                          exist, passed by reference through the GameEngineData callbacks. See
                          "Architecture notes" below
    AssetManager.h/.cpp   Owns an SDL_Texture cache and a TTF_Font cache (each unique_ptr + custom
                          deleter, fonts keyed by file path + point size since a TTF_Font is
                          rasterized at a fixed size), constructed with the renderer it belongs to.
                          Calls TTF_Init/TTF_Quit itself since LoadFont is the only thing in Engine
                          that needs SDL_ttf up
    InputManager.h/.cpp   Keyboard/mouse state plus gamepad connect/disconnect notifications
                          (OnGamepadConnected/OnGamepadDisconnected), constructed with the
                          SDLEventDispatcher it subscribes to. Also holds the UI pointer-claim flag
                          (SetPointerClaimed/IsPointerClaimed) that Canvas sets and
                          IsMouseButtonPressed/Released/JustPressed respect by default, see
                          "Architecture notes" below
    Camera.h/.cpp         Screen-space zoom/scale for pixel art, constructed with the window/renderer it targets
    DebugOverlay.h/.cpp   Dear ImGui setup/teardown/frame bracketing, shared by Game (and a future
                          Editor). Debug builds only (ENGINE_WITH_DEBUG_UI); every method is a no-op
                          in Release, see "Architecture notes" below
    GameEvent.h           Generic multicast delegate (GameEvent<Args...>), used for engine-wide events
    Log.h                 Minimal SDL_Log-based macros (ENGINE_LOG_INFO/WARN/ERROR); Core.h's
                          ENGINE_ASSERT/ENGINE_CORE_ASSERT build on top of these
    math/Vector2D.h        Basic 2D vector type
    sdl/
      SDLEventDispatcher.h/.cpp  Central SDL_PollEvent loop; per-event-type GameEvent broadcast
      SDLEventHandler.h/.cpp     RAII wrapper that registers/unregisters a listener on a dispatcher
    ecs/
      Entity.h              Entity = packed {index, generation} uint32_t handle. MAX_ENTITIES = 1024
      ComponentType.h/.cpp  GetComponentId<T>(): assigns each component type a stable id automatically
                            (keyed by RTTI type identity, routed through one exported function so it
                            stays correct across the Engine/Game DLL boundary, see "Architecture notes")
      ComponentArray.h      Template ComponentArray<T>: plain std::vector<T>, one per component type,
                            indexed directly by entity index. Presence is tracked entirely by
                            EntityManager's ComponentMask, not here
      EntityManager.h/.cpp  CreateEntity/DestroyEntity/AddComponent<T>/GetComponent<T>/RemoveComponent<T>
                            (fully type-safe, no manual component ids anywhere), plus ForEach<T...>(fn)
                            for iterating all entities that have a given set of components, and
                            RunSystems for the plain std::function<void(float)> systems list
      component/             TransformComponent, VelocityComponent, SpriteComponent, AnimationComponent
      system/                AnimationSystem (currently a stub, Update() is empty)
    ui/
      Widget.h/.cpp          Base of the retained UI tree: Measure/Arrange (two-pass layout)/Render,
                            a Visible flag, and Rect/Size/Alignment. A bare Widget is a valid
                            invisible spacer, not abstract. See "Architecture notes" below
      BoxContainer.h/.cpp    Shared single-axis flex-style layout (Auto/Fill sizing per slot, cross-
                            axis Alignment, Padding) that HorizontalBox/VerticalBox both use
      HorizontalBox.h/.cpp, VerticalBox.h/.cpp  Thin BoxContainer subclasses; just say which axis is main
      Panel.h/.cpp           Solid-color rect widget; the simplest renderable widget
      Text.h/.cpp            Renders a string via SDL_ttf. Measure only needs the TTF_Font
                            (TTF_SizeUTF8, no renderer involved), the actual SDL_Texture is built
                            lazily the first time Render() runs (and rebuilt on any SetFont/
                            SetText/SetColor change), since Render is the only pass with a
                            renderer to build it with. Draws centered in whatever rect Arrange
                            gives it, not stretched, so it keeps its natural glyph size
      Image.h/.cpp           Renders a non-owning SDL_Texture* (from AssetManager::LoadTexture,
                            same convention as Game.cpp's own texture fields) stretched into
                            whatever rect Arrange gives it
      Button.h/.cpp          Clickable rect wrapping an optional Content child widget (a Text
                            label, an Image, a Panel, a whole layout); Pressed/Released/Clicked
                            GameEvents, hover/press color states, BlocksInput on by default.
                            SetBackgroundImage swaps the flat-color fill for a stretched texture
                            (darkens on press instead of the hover/press color swap, since texture
                            tinting can only darken). Background and Content are independent, so a
                            button can have both an image background and a text label at once. See
                            "Architecture notes" below
      Canvas.h/.cpp          Owns an ordered list of root widgets (later = drawn on top), each laid
                            out to fill the screen. Three separate entry points rather than one
                            "update" call: UpdateLayout (Measure/Arrange), ProcessInput (hit-test,
                            hover/press/click dispatch, claims the pointer), Render (draw only).
                            EngineContext::UICanvas, driven automatically by GameEngine every frame,
                            see "Architecture notes" below for why layout/input run before gameplay

Game/                   Game executable (links against the Engine DLL)
  Game.h/.cpp            The file to actually write a game in: Game::Startup/Update/Draw/Shutdown, the
                          four callbacks described in Game.h and under "Architecture notes" below.
                          Currently a minimal example: one entity with a TransformComponent and a
                          VelocityComponent, a MovementSystem, a loaded sprite drawn at the entity's
                          position, and (guarded by #ifdef ENGINE_WITH_DEBUG_UI, see "Architecture
                          notes") an ImGui demo window to show ImGui is already available in Debug.
  Bootstrap.cpp           Implements Engine::CreateGameEngineData(): running registered systems each
                          tick, then forwarding to Game::Startup/Update/Draw/Shutdown. Framework glue,
                          not game-specific; there's no need to open this file to make a game.
  assets/images/          Game content (bomb.png example sprite)

cmake/
  Dependencies.cmake     FetchContent for SDL2, SDL2_image, SDL2_ttf (pinned release tags). A fresh
                        clone builds these from source automatically, no manual SDK folder needed for them
CMakeLists.txt          Top-level: C++23, CMAKE_BUILD_TYPE defaults to Debug (only if not already set),
                        BUILD_DIR tracks CMAKE_BINARY_DIR, adds Engine then Game subdirs. The compiler is
                        picked normally (via CMakePresets.json, -DCMAKE_CXX_COMPILER, or generator
                        default), not forced. Also swaps in the real Windows SDK rc.exe in place of
                        LLVM's llvm-rc for the Clang toolchain (see "Known rough edges" for why); this has
                        to happen, along with the RC rule/include-dir plumbing that goes with it, before
                        project(), because that's when CMake's Windows-Clang platform module auto-detects
                        and locks in an RC compiler
CMakePresets.json       x64-debug/x64-release (Ninja + Clang, resolved via PATH) and x64-debug-msvc
                        (Visual Studio generator) configure/build presets. x64 only, 32-bit isn't
                        targeted, ARCH still exists as a CMake variable if a 32-bit build is ever needed
.vscode/                tasks.json (Configure/Build Debug|Release x64, via `cmake --preset`),
                        launch.json (lldb debug configs matching each build preset's output dir),
                        c_cpp_properties.json (clang toolchain, for IntelliSense only)
```

## Build & run

Requires Dear ImGui sources, LLVM/Clang, Ninja, CMake ≥3.20 (for `CMakePresets.json`), all discoverable via
the `CMAKE_PREFIX_PATH` env var (e.g. `C:\Code\sdks`, expected to contain an `imgui/` subfolder, that's the
only manual dependency left) and `PATH` including the LLVM `bin` dir. SDL2/SDL2_image/SDL2_ttf are fetched
and built automatically by `cmake/Dependencies.cmake`, no manual download/placement needed for those. Dear
ImGui itself is only compiled into Debug builds (see "Architecture notes"), but its sources still need to
be present under `CMAKE_PREFIX_PATH` for the Debug preset regardless.

```
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Other presets: `x64-release` (Clang/Ninja), and `x64-debug-msvc` (MSVC via the Visual Studio generator, a
starting point for building with MSVC instead of Clang; VS Code debug wiring for it isn't set up yet, only
the CMake side).

VS Code: use the "Build Debug/Release x64" tasks (they just wrap the presets above) and matching lldb
launch configs in `.vscode/`.

## Architecture notes for future work

- **Engine/Game split is deliberate and should stay strict.** Engine = platform/SDL wrapping, ECS
  infrastructure, asset/input/camera/debug-UI management, reusable systems. Game = actual gameplay:
  entities, components' *content*, game-specific systems, whatever debug windows a specific game
  wants to build with the ImGui API Engine already set up. The long-term idea is Game could
  eventually be replaced or driven by a scripting layer (Lua/Odin/etc. floated as options) without
  touching Engine, so avoid leaking game-specific concepts into Engine code.
- **GameEngineData is the seam between the two.** It's a set of four `std::function` callbacks
  (Startup/Update/Draw/Shutdown, all taking an `Engine::EngineContext &`, Update also takes the fixed
  timestep as a float) that Game populates via `Engine::CreateGameEngineData()`, called from
  `EntryPoint.h`'s `main()`. This is effectively the plugin boundary, keep it in mind if/when an editor
  mode or hot-reloadable game logic gets added.
- **Game/ itself is split in two, for the same reason Engine/Game is split.** `Game/Game.cpp` is the
  only file meant to be edited to make a game: `Startup`/`Update`/`Draw`/`Shutdown`, no SDL setup
  visible, and (in Debug builds) Dear ImGui already set up so `ImGui::` calls just work.
  `Game/Bootstrap.cpp` is the framework glue that makes that possible: it implements
  `Engine::CreateGameEngineData()`, forwarding to the four `Game::` callbacks. Nothing in
  Bootstrap.cpp is specific to any particular game; a newcomer should never need to open it.
- **Dear ImGui lives in Engine, not Game, and only in Debug builds.** `Engine::DebugOverlay`
  (`Engine/src/DebugOverlay.h`) owns Dear ImGui's setup/teardown and per-frame `NewFrame()`/`Render()`
  bracketing; `GameEngine::Update()` calls it automatically around every `Draw()`, so `Game::Draw` (or
  a future Editor's own draw code) never needs to touch ImGui setup, just call `ImGui::` functions
  directly. Compiled and linked only for Debug (`ENGINE_WITH_DEBUG_UI`, defined via
  `$<CONFIG:Debug>` in `Engine/CMakeLists.txt`, not a `CMAKE_BUILD_TYPE` string check, that breaks for
  the multi-config MSVC preset), Dear ImGui's own source is never compiled into a Release binary at
  all. `DebugOverlay` is still always declared and always an `EngineContext` member either way, so
  `EngineContext`/`GameEngine` themselves stay free of `#ifdef`, its methods are simply no-ops in
  Release; only code that calls the raw `ImGui::` API directly (like `Game.cpp`'s demo window) needs
  an `#ifdef ENGINE_WITH_DEBUG_UI` guard, since that's inherently a compile-time thing, no plumbing
  can make an unlinked library's functions conditionally callable at runtime.

  Three reasons it lives in Engine specifically: (1) it's shared, reusable infrastructure a future
  Editor target should get for free rather than reimplementing, (2) Dear ImGui keeps process-wide
  global state (its "current context"), so it can only safely exist as one compiled copy, if both
  Engine and Game compiled their own separate copies of ImGui's source, each would have its own
  disconnected global state and calls would silently operate on the wrong one, so it has to be
  compiled into exactly one binary and shared across the DLL boundary, and (3) as a consequence of
  (2), the Engine DLL needs `WINDOWS_EXPORT_ALL_SYMBOLS ON` (Dear ImGui has no `dllexport`
  annotations of its own the way Engine's own `ENGINE_API` classes do).
- **Fixed-timestep loop** lives in `GameEngine::Update()` (60Hz, max 5 frame-skips), already accumulating
  `Lag`. `Context->World.RunSystems(FixedTimeStep)` runs there too, automatically, right before
  `EngineData->Update(...)`, the same "Engine drives it, Game/Bootstrap don't have to" pattern used for
  `DebugOverlay`'s frame bracketing. Rendering is currently once per real frame regardless of catch-up
  updates, no interpolation.
- **ECS is homegrown and intentionally simple**, not archetype/SoA-based. Entities are index+generation
  handles; components live in one `ComponentArray<T>` (a plain `std::vector<T>`) per component type,
  indexed directly by entity index, with presence tracked separately by a `ComponentMask` bitset per
  entity, so `ComponentArray<T>` itself needs no per-slot "has value" bookkeeping. Component ids are
  assigned automatically per type via `GetComponentId<T>()` rather than hand-maintained constants;
  `EntityManager::ForEach<T...>(fn)` covers the common "run this over every entity with these
  components" case so systems don't hand-roll the iteration. Systems themselves are still just
  `std::function<void(float)>` run in registration order every frame, no dependency ordering, no
  parallelism. Good enough for a handful of entities, not yet the "data-driven, lots of enemies,
  well-optimized" version described as the ambition (that would mean archetypes/SoA layout and
  parallel system scheduling), treat current ECS as a solid middle step, not the end state.
- **`GetComponentId<T>()` can't just be a static counter inside the template function.** Component
  types get used from both Engine and Game code, which are separate binaries (the Engine DLL and the
  Game exe), and each binary gets its own copy of a template-local static, so a naive per-type counter
  could hand out different ids for the same type depending on which binary happened to instantiate it
  first. `ComponentType.cpp` routes the actual id allocation through one exported function (keyed by
  `std::type_index`) that lives only in the Engine DLL, so it stays a single source of truth regardless
  of which side asks first. Worth remembering if any other cross-DLL "identify a type at runtime" need
  comes up later, same problem, same fix.
- **Style**: Unreal-esque, PascalCase for types/functions/members (no `m_`/`_` prefixes), no Hungarian
  notation, braces on their own line, `Engine::`/`Game::` namespaces rather than prefixes. Stated preference
  to avoid templates where reasonably possible and to minimize std library usage; current code does use
  STL fairly heavily (vector, unordered_map, function, any, optional, bitset) as a pragmatic starting point,
  treat this as a direction to move in over time, not a rule already achieved.
- **No singletons.** AssetManager, InputManager, Camera, and the ECS `World` (an `EntityManager`) are
  ordinary constructed objects owned by `EngineContext`, not `GetInstance()` globals. They take what
  they need (Renderer, SDLEventDispatcher, Window/Renderer) as constructor parameters instead of
  two-phase `Initialize()` calls. This mirrors how Window/Renderer were already passed explicitly
  through `GameEngineData` rather than reached for globally, and keeps the door open for multiple
  contexts later (an editor viewport alongside a play viewport, for instance) without any global state
  to fight over.

  Rule for what belongs in `EngineContext`: it holds state (not a stateless utility like `Log.h`'s
  macros), and there should structurally only ever be one of it in a running engine, not just "only one
  today because nothing needs more yet" (a future AudioManager fits; a physics world that might want
  per-scene copies later doesn't). The `World` fits this: it's a generic, reusable container, the same
  category as `AssetManager`, even though the entities/components it happens to hold at runtime are
  entirely game-specific. What stays out is actual gameplay logic/business state itself (a score counter,
  a "current level" enum, anything that's really *your game*, not infrastructure for building one), that
  belongs in `Game.cpp`. When something does fit `EngineContext`, follow the same pattern: construct it
  with its dependencies, own it there.
- **GameEvent<Args...>** is the multicast-delegate/observer building block used throughout (SDL dispatch,
  InputManager, ImGui SDL event hookup). New cross-cutting notifications should probably go through this
  rather than a bespoke callback list.
- **The UI system (`Engine::UI`) is retained-mode, deliberately not ImGui**, and lives in Engine so
  a future Editor can use the exact same widgets/layout Game does, not reimplement its own. ImGui is
  immediate-mode (redescribe everything every frame), which is exactly why it's great for debug
  tools and a poor fit for a styled, animated game UI with data binding, there's nothing persistent
  to bind *to*. The retained tree also means rendering doesn't need Game to redeclare anything each
  frame: `GameEngine::Update()` renders `Context->UICanvas` automatically, right after the game
  world and before `DebugOverlay` (which stays topmost, for debugging), at 1:1
  screen scale via the same `MainCamera.ResetScale()` trick `DebugOverlay` already uses, decoupled
  from the pixel-art zoom. `Game::Startup` builds the widget tree once; later code just mutates it
  (toggle `Visible`, change a color, etc.) rather than re-declaring it.

  Layout is a single-axis flex-style model (`HorizontalBox`/`VerticalBox`, standard two-pass
  Measure-then-Arrange, the same shape Slate/WPF/Flutter all use): each child sits in a slot with a
  `SizeRule` (`Auto` = the child's own measured size, `Fill(weight)` = a share of whatever's left)
  for the main axis, and one `Alignment` (`Start`/`Center`/`End`/`Fill`) for the cross axis, that
  same alignment is also how a menu gets centered on screen, no separate anchor/positioning system
  needed. `Canvas` holds a *list* of root widgets rather than a single root specifically so a future
  CommonUI-style layer stack (HUD, then an inventory screen on top of it) doesn't need a redesign
  later, later root = drawn on top, that's most of what layering needs already.

  Hit-testing, hover/press/click events, and an input-claim mechanism are in place, so UI and
  gameplay read the same `InputManager` state (no separate Game/UI input pipelines like Unreal's),
  but UI wins whenever a click lands on a widget. `Widget::HitTest(x, y)` returns the topmost widget
  that opts in via `BlocksInput` (off by default, so decorative widgets like a bare `Panel` don't
  accidentally eat input; `Button` turns it on in its constructor) or `nullptr`;
  `BoxContainer::HitTest` recurses into its children in reverse order (last-added/drawn-on-top gets
  first refusal) before falling back to itself. `Canvas::ProcessInput` runs that hit-test against
  every root (later roots first, same top-to-bottom-visually order as `HitTest` within a container),
  calls `InputManager::SetPointerClaimed()` with the result, then dispatches
  `OnPointerEnter`/`OnPointerLeave`/`OnPointerDown`/`OnPointerUp` on whatever widget was hit. `Button`
  builds `OnClicked` on top of that: a click only fires if `OnPointerUp` happens while still hovering
  the same widget that was pressed, so dragging off and releasing elsewhere doesn't count.
  `InputManager` itself knows nothing about widgets or `Canvas`, it just exposes a plain
  `PointerClaimed` bool (`SetPointerClaimed`/`IsPointerClaimed`) that `Canvas` writes to; gameplay
  reads it indirectly, since `IsMouseButtonPressed`/`Released`/`IsMouseButtonJustPressed` all take a
  `RespectUIClaim` parameter (defaults `true`) and return false while the pointer is claimed, so
  gameplay code gets "UI blocks clicks under it" for free without checking `IsPointerClaimed()`
  manually. `Canvas`'s own hit-testing bypasses the claim (passes `false`) since it's the code
  deciding the claim in the first place, it needs the real button state regardless of any earlier
  claim.

  This is why `GameEngine::Update()` calls `Context->UICanvas.UpdateLayout(Renderer)` then
  `Context->UICanvas.ProcessInput(Context->Input)` before the fixed-update loop runs any gameplay
  code, not from the render pass (which still only calls `Canvas::Render`, after gameplay `Update`,
  same as before): gameplay reads `IsPointerClaimed`/the `RespectUIClaim`-gated input calls during its
  own `Update`, so the claim for this frame has to already be decided by then, or it would be one
  frame stale.

  `Button` is a clickable rect wrapping an optional `Content` child widget (`Text`, `Image`, a
  `Panel`, or a whole layout, nothing about `Button` is specific to any of them) rather than
  separate `IconButton`/`TextButton` types, so new content widgets slot in without a rewrite. It
  exposes `OnPressed`/`OnReleased`/`OnClicked` as `GameEvent<>`s and cycles between normal/hovered/
  pressed colors based on the pointer events above, or (via `SetBackgroundImage`) a stretched
  texture instead of a flat color; `Content` and the background are independent, so a button can
  have both a background image and a `Text` label at once.

  `Text` and `Image` both need something layout (`Measure`) doesn't have access to: `Text` needs a
  loaded `TTF_Font*` (from `AssetManager::LoadFont`) purely to ask SDL_ttf how big the rendered
  string would be (`TTF_SizeUTF8`, no renderer or actual rasterization involved), `Image` needs a
  loaded `SDL_Texture*` (from `AssetManager::LoadTexture`) to ask its pixel size. Neither widget
  owns the asset itself, both take a non-owning pointer that outlives the widget, same convention
  `Game.cpp` already used for its own `PlayerTexture`. `Text` does own one thing: the `SDL_Texture`
  it rasterizes its string into. Since that requires an `SDL_Renderer`, which only `Render()`
  receives (not `Measure()`), the texture is built lazily on first `Render()` and rebuilt whenever
  `SetFont`/`SetText`/`SetColor` change, rather than eagerly in a setter.

  Next up: the rest of the widget set (Checkbox/Dropdown/Scalar), then an MVVM binding layer built
  on `GameEvent` rather than reflection/macros (no Blueprint-equivalent here needing runtime
  property discovery, so no need for that machinery). Drag-and-drop and the layer stack are noted
  for later, not being built now.
- **Gamepad connect/disconnect** (`InputManager::OnGamepadConnected()`/`OnGamepadDisconnected()`) hides
  an SDL2 quirk: `SDL_CONTROLLERDEVICEADDED`'s `which` field is a transient device index, but
  `SDL_CONTROLLERDEVICEREMOVED`'s `which` is the stable `SDL_JoystickID`, two different kinds of value
  in the same-looking field depending on which event it is. `InputManager` opens the controller on
  ADDED (also required to receive any further events for it at all), derives the real
  `SDL_JoystickID` from the now-open `SDL_GameController*`, and broadcasts that same stable id on both
  connect and disconnect, so listeners never see the index/id distinction. Opened controllers are
  tracked in a map keyed by that id and explicitly `SDL_GameControllerClose()`'d, individually on
  disconnect or all of them in `~InputManager()`. Reading actual button/axis state isn't implemented
  yet, this is connect/disconnect notification only.

## Known rough edges / things to be aware of (not yet fixed, don't assume otherwise)

- `Engine::UI` still has no `Checkbox`/`Dropdown`/`Scalar` widgets, and no MVVM/data binding layer.
  `Widget`/`BoxContainer`/`HorizontalBox`/`VerticalBox`/`Panel`/`Text`/`Image`/`Button`/`Canvas`
  cover layout, rendering, hit-testing, click/hover events, text, and images already, see
  "Architecture notes" for the intended next phases.
- `Game.cpp`'s label-button demo loads `C:/Windows/Fonts/arial.ttf` directly, since the repo has no
  bundled `.ttf` yet. That path only resolves on Windows machines with Arial installed there, swap
  it for a real font under `Game/assets/fonts/` (loaded via `Context.Assets.LoadFont`, same relative-
  path convention as `Context.Assets.LoadTexture("assets/images/bomb.png")`) once one is added.
- `Engine/src/ecs/system/AnimationSystem.h` is a stub (`Update` does nothing); `AnimationComponent` and
  `SpriteComponent` exist but aren't wired into any running system yet, `Game.cpp` calls `SDL_RenderCopy`
  directly in `Draw` (reading its position from `TransformComponent`, but not the sprite/frame data from
  `SpriteComponent`) rather than there being a generic sprite-rendering system.
- A `dllexport`ed class's `= default` constructor, if written inline in the class body, doesn't get its
  symbol emitted unless something inside the Engine DLL's own compiled sources actually calls it, which
  breaks any consumer in Game.exe trying to link against it. `VelocityComponent` and `TransformComponent`
  avoid this by declaring their constructors in the header and defining them (still `= default` where
  applicable) in a matching `.cpp`. Follow the same pattern for any future `ENGINE_API` component/class
  whose special members might otherwise never get used from inside Engine's own sources.
- `x64-debug-msvc` (CMakePresets.json) only covers the CMake configure/build side of an MSVC build, there's
  no VS Code launch config or `cppvsdbg` debug wiring for it yet.
- Building SDL2_ttf's vendored FreeType surfaces a real toolchain quirk on the Clang path: LLVM's `llvm-rc`
  (which CMake picks by default for Clang-on-Windows RC/resource compilation) has a bug parsing non-ASCII
  bytes in `.rc` files (FreeType's `ftver.rc` has a copyright symbol), so the top-level `CMakeLists.txt`
  swaps in the real Windows SDK `rc.exe` instead, including its own compile rule (bypassing CMake's
  default Clang+llvm-rc pipeline, which pipes `.rc` files through `clang -E` in a format only llvm-rc
  understands) and explicit `um`/`shared` SDK include dirs (rc.exe has no compiler-driver auto-discovery
  of `windows.h` etc. the way Clang does). This assumes a Windows SDK is installed under the standard
  `Program Files (x86)/Windows Kits/10` location, which is already an implicit prerequisite for any
  Windows C++ toolchain (Clang-on-Windows needs it too, just discovers it more transparently for C/C++
  than for RC).

## Future direction (stated intent, not yet implemented, for context, not as a todo list)

- Optional scripting layer for gameplay code (Lua, Odin, or similar) sitting where `Game/` is now, keeping
  `Engine/` as the stable C++ core.
- A separate Editor project (alongside Engine and Game) using Dear ImGui for its UI, already shared,
  reusable infrastructure via `Engine::DebugOverlay` rather than something to set up again, to edit
  game logic/scene data, with a separate "build the actual game" path that skips the editor.
- An archetype/SoA-based ECS for bulk entity spawning (e.g. many enemies) with cache-friendly iteration
  and parallel system scheduling, current `ComponentArray<T>`-per-type design is a solid middle step,
  not that end state.
- Full MSVC support: `CMakePresets.json` has a starting `x64-debug-msvc` configure/build preset, but VS Code
  debug (breakpoints, `cppvsdbg`) isn't wired up for it yet.
