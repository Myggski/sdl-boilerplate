#pragma once

#include "Core.h"
#include <cstdint>
#include <memory>
#include <utility>

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

  // Inset from a widget's own rect on each side (e.g. Button::SetContentPadding). Use designated
  // initializers ({.Left = 12.0f, .Top = 8.0f, .Right = 12.0f, .Bottom = 8.0f}) at the call site
  // rather than relying on field order.
  struct Padding
  {
    float Left = 0.0f;
    float Top = 0.0f;
    float Right = 0.0f;
    float Bottom = 0.0f;
  };

  // The engine's own color type: the public UI API (widget setters, Theme's constants) uses this,
  // never SDL_Color directly, and Engine::UI::Color is all game code ever sees through the normal
  // #include "Engine.h" umbrella. Most SDL calls (SDL_SetRenderDrawColor, SDL_SetTextureColorMod)
  // take separate r/g/b/a bytes and need no conversion at all; the one real exception (SDL_ttf's
  // TTF_RenderUTF8_Blended, see Text.cpp) converts via ToSDLColor() in ColorConversion.h, an
  // engine-internal header not included from Engine.h, so SDL_Color itself never leaks out here.
  struct Color
  {
    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
    uint8_t A = 255;
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
    // X/Y are the pointer's position at the moment of the press, in screen pixels (same space as
    // ComputedRect); Scalar uses this to jump straight to the value under the click, like a
    // slider, rather than only ever adjusting relative to wherever it happened to start.
    virtual void OnPointerDown(float X, float Y);
    virtual void OnPointerUp(bool StillHovered);

    // Fired every frame the pointer stays held down on this widget, after the initial
    // OnPointerDown and before the eventual OnPointerUp (never on the same frame as either). X/Y
    // are the pointer's current absolute position (same space as ComputedRect/OnPointerDown);
    // DeltaX/DeltaY are movement since last frame, for widgets that want relative adjustment
    // instead. No-op by default; Scalar uses the absolute position so drag always matches
    // exactly where the pointer is (same mapping OnPointerDown uses), rather than accumulating
    // per-frame deltas at a separately-tuned sensitivity, which would drift out of sync with it.
    virtual void OnPointerDrag(float X, float Y, float DeltaX, float DeltaY);

    const Rect &GetComputedRect() const { return ComputedRect; }

    bool Visible = true;

    // Opt-in: does this specific widget claim (and block passthrough of) input landing on it.
    // Off by default so decorative widgets (Panel, plain layout boxes) don't silently start
    // eating clicks just by existing; Button defaults this on.
    bool BlocksInput = false;

  protected:
    Rect ComputedRect{};
  };

  // Builder-style construction: every concrete widget's own setters return a self pointer (e.g.
  // Button *SetColors(...)) so they chain, CreateWidget<T>() just saves writing
  // std::make_unique<T>() at every call site.
  //
  //   std::unique_ptr<Button> NewButton = CreateWidget<Button>();
  //   NewButton->SetDesiredSize({48.0f, 48.0f})->SetColors(Normal, Hovered, Pressed);
  //   Button *Result = Toolbar->AddSlot(std::move(NewButton), SizeRule::Auto, 1.0f, Alignment::Center, 8.0f);
  //
  // The chain can't extend into AddSlot/AddRoot/SetContent itself: by the time a widget can
  // return a self pointer for the next call in the chain, it no longer has access to the
  // std::unique_ptr that owns it (nothing does, other than the caller's own variable), so there
  // is no safe way for the widget to hand its own ownership off to a parent from inside a
  // chained call. Attaching to a parent stays a separate, explicit std::move, same as before.
  // AddSlot/AddRoot/SetContent are all templated on the widget type they're given, though, so
  // that std::move still hands back a pointer of the exact concrete type passed in (Button*, not
  // Widget*), no separate NewButton.get() capture and no manual cast needed to use Result above.
  template <typename T, typename... Args>
  std::unique_ptr<T> CreateWidget(Args &&...ConstructorArgs)
  {
    return std::make_unique<T>(std::forward<Args>(ConstructorArgs)...);
  }
}
