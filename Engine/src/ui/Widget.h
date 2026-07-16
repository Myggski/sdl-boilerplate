#pragma once

#include "Core.h"

struct SDL_Renderer;

namespace Engine::UI
{
  struct Size
  {
    float Width = 0.0f;
    float Height = 0.0f;
  };

  struct Rect
  {
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
  };

  // How a widget is positioned within the space its slot has to offer, along whichever axis
  // isn't already fully determined by box layout's Auto/Fill sizing (see BoxContainer.h).
  enum class Alignment
  {
    Start,
    Center,
    End,
    Fill
  };

  // Base of the retained UI tree. A plain Widget with no children/rendering is a valid,
  // invisible spacer, deliberately not abstract. Layout is two passes, standard for
  // flex-style/retained UI: Measure (bottom-up, "how much space do you want") then Arrange
  // (top-down, "here's the space you actually get"); Render happens after both, using whatever
  // Arrange last computed. Recomputed every frame for now, no dirty-flag caching; this is a
  // start-menu/settings-menu scale UI, not a performance concern yet.
  class ENGINE_API Widget
  {
  public:
    Widget();
    virtual ~Widget();

    Widget(const Widget &) = delete;
    Widget &operator=(const Widget &) = delete;

    virtual Size Measure(Size AvailableSize);
    virtual void Arrange(Rect FinalRect);
    virtual void Render(SDL_Renderer *Renderer);

    // Returns the topmost widget at (X, Y) that wants input (BlocksInput, and only reachable
    // while Visible), or nullptr if nothing there claims it. The default checks this widget's own
    // ComputedRect; BoxContainer overrides it to check children first, in reverse render order
    // (whatever's drawn last/on top gets first refusal), falling back to itself.
    virtual Widget *HitTest(float X, float Y);

    // No-op by default; Button (and anything else that wants pointer interaction) overrides these.
    // Canvas calls them during its input pass, not meant to be called directly by game code.
    virtual void OnPointerEnter();
    virtual void OnPointerLeave();
    virtual void OnPointerDown();
    virtual void OnPointerUp(bool StillHovered);

    const Rect &GetComputedRect() const { return ComputedRect; }

    bool Visible = true;

    // Opt-in: does this specific widget claim (and block passthrough of) input landing on it.
    // Off by default so decorative widgets (Panel, plain layout boxes) don't silently start
    // eating clicks just by existing; Button defaults this on.
    bool BlocksInput = false;

  protected:
    Rect ComputedRect{};
  };
}
