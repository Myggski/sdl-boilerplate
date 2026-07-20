#pragma once

#include "Core.h"
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <cstdint>
#include <memory>
#include <string>
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

  // The engine's own color type; game code sees this, never SDL_Color, through the Engine.h
  // umbrella. Converts to SDL_Color only where actually needed (see SDLConversions.h).
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

  // Base of the retained UI tree. A plain Widget with no children/rendering is a valid, invisible
  // spacer - not abstract. Layout is two passes: Measure (bottom-up, "how much space do you
  // want") then Arrange (top-down, "here's what you get"); Render reuses the last Arrange result.
  // Recomputed every frame, no dirty-flag caching - fine at this UI's scale.
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

    // No-op by default; Canvas calls these during its input pass, not meant to be called
    // directly by game code.
    virtual void OnPointerEnter();
    virtual void OnPointerLeave();
    // X/Y are the pointer's position in screen pixels (same space as ComputedRect).
    virtual void OnPointerDown(float X, float Y);
    virtual void OnPointerUp(bool StillHovered);

    // Fired every frame the pointer stays held down, between OnPointerDown and OnPointerUp. X/Y
    // are the current absolute position; DeltaX/DeltaY are movement since last frame, for
    // widgets that want relative adjustment instead.
    virtual void OnPointerDrag(float X, float Y, float DeltaX, float DeltaY);

    // No-op by default; Canvas calls these when keyboard focus moves to/from this widget (only
    // reachable with WantsFocus set). Modifiers is the live modifier state at the moment Canvas
    // forwards the key.
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnTextInput(const std::string &Text);
    virtual void OnKeyDown(SDL_Scancode PressedKey, SDL_Keymod Modifiers);

    const Rect &GetComputedRect() const { return ComputedRect; }

    bool Visible = true;

    // Opt-in: does this specific widget claim (and block passthrough of) input landing on it.
    // Off by default so decorative widgets (Panel, plain layout boxes) don't silently start
    // eating clicks just by existing; Button defaults this on.
    bool BlocksInput = false;

    // Opt-in: can this widget hold keyboard focus. Off by default; TextInput sets this on.
    // Canvas transfers focus to a clicked widget only if this is true.
    bool WantsFocus = false;

  protected:
    Rect ComputedRect{};
  };

  // Builder-style construction: concrete widgets' setters return a self pointer so they chain;
  // this just saves writing std::make_unique<T>() at every call site.
  //
  //   std::unique_ptr<Button> NewButton = CreateWidget<Button>();
  //   NewButton->SetDesiredSize({48.0f, 48.0f})->SetColors(Normal, Hovered, Pressed);
  //   Button *Result = Toolbar->AddSlot(std::move(NewButton), SizeRule::Auto, 1.0f, Alignment::Center, 8.0f);
  //
  // The chain can't extend into AddSlot itself (ownership transfer needs an explicit std::move),
  // but AddSlot/AddRoot/SetContent are templated on the widget type, so they still hand back a
  // pointer of the concrete type passed in.
  template <typename T, typename... Args>
  std::unique_ptr<T> CreateWidget(Args &&...ConstructorArgs)
  {
    return std::make_unique<T>(std::forward<Args>(ConstructorArgs)...);
  }
}
