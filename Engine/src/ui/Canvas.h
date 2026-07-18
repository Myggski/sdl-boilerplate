#pragma once

#include "Core.h"
#include "Widget.h"
#include <memory>
#include <vector>

struct SDL_Renderer;

namespace Engine
{
  class InputManager;
}

namespace Engine::UI
{
  // Owns the UI tree(s) for the whole game: an ordered list of independent root widgets, each
  // laid out to fill the screen and rendered in order (later root = drawn on top). Holding a
  // list rather than a single root is deliberate groundwork for a future CommonUI-style layer
  // stack (e.g. HUD, then a menu on top of it), even though there's no layer concept yet, just
  // ordering.
  //
  // Three separate steps, called at different points in the frame (see GameEngine::Update()):
  // UpdateLayout (Measure+Arrange, no drawing) has to happen before ProcessInput (hit-testing
  // needs fresh rects) which has to happen before gameplay reads input (so this frame's claim is
  // already set, not last frame's); Render happens later, in the render pass, reusing the same
  // rects UpdateLayout already computed rather than redoing layout.
  class ENGINE_API Canvas
  {
  public:
    Canvas();
    ~Canvas();

    Canvas(const Canvas &) = delete;
    Canvas &operator=(const Canvas &) = delete;

    // Takes ownership of Root; returns a non-owning pointer of the same concrete type that was
    // passed in, same reasoning as BoxContainer::AddSlot (see its comment).
    template <typename T>
    T *AddRoot(std::unique_ptr<T> Root)
    {
      T *Result = Root.get();
      Roots.push_back(std::move(Root));
      return Result;
    }

    void UpdateLayout(SDL_Renderer *Renderer);
    void ProcessInput(InputManager &Input);
    void Render(SDL_Renderer *Renderer);

  private:
    std::vector<std::unique_ptr<Widget>> Roots;

    // Persist across frames so ProcessInput can detect enter/leave, press/release, and drag
    // (mouse-moved-while-held) transitions.
    Widget *HoveredWidget{nullptr};
    Widget *PressedWidget{nullptr};
    Widget *FocusedWidget{nullptr};
    float LastMouseX{0.0f};
    float LastMouseY{0.0f};
  };
}
