#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"
#include "GameEvent.h"
#include <memory>

struct TTF_Font;

namespace Engine
{
  class AssetManager;
}

namespace Engine::UI
{
  class Text;

  // A slider-style numeric box: with a range set (SetRange), clicking or dragging anywhere in it
  // jumps the value straight to whatever position the pointer is at, the same mapping both times
  // (position 3/20 of the way across a -10..10 range is always -7, whether that's a fresh click
  // or mid-drag), rather than accumulating per-frame drag deltas at a separately-tuned
  // sensitivity that could drift out of sync with where the pointer actually is. Without a range,
  // there is no defined position-to-value mapping, so it falls back to plain relative dragging
  // (SetDragSensitivity) instead, same idea as ImGui's DragFloat. There is no keyboard
  // text-input/focus system yet, so typing a value directly isn't an option either way. Composes
  // a Text label internally to show the formatted value, same box-plus-label shape as Button,
  // reusing Text's font/render handling rather than duplicating it.
  class ENGINE_API Scalar : public Widget
  {
  public:
    Scalar();
    ~Scalar() override;

    Scalar *SetFont(TTF_Font *NewFont);

    // Convenience: loads the font (and applies the color) from Theme::TextStyle in one call,
    // rather than the caller pre-loading a TTF_Font* and calling SetFont/SetTextColor separately.
    Scalar *SetFont(Engine::AssetManager &Assets, Theme::TextStyle Style = {});

    Scalar *SetValue(float NewValue);
    float GetValue() const { return Value; }

    // Min > Max (the default, 0/0) means unclamped.
    Scalar *SetRange(float NewMin, float NewMax);

    // Snaps Value to the nearest multiple of Step (e.g. Step=1 for -10..10, Step=10 for
    // 100..300). 0 (the default) means continuous/unsnapped.
    Scalar *SetStep(float NewStep);

    // Raw value change per pixel of horizontal drag. Only used when no range is set (see class
    // comment); a ranged Scalar derives its own effective sensitivity from range/width instead,
    // to stay consistent with click-to-position.
    Scalar *SetDragSensitivity(float NewSensitivity);

    Scalar *SetColors(Color Normal, Color Hovered, Color Pressed);

    // Fill bar showing where Value sits between MinValue/MaxValue (only drawn when a range is
    // set via SetRange), same idea as a slider's filled track.
    Scalar *SetFillColor(Color NewColor);

    Scalar *SetTextColor(Color NewColor);
    Scalar *SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Arrange(Rect FinalRect) override;
    void Render(SDL_Renderer *Renderer) override;

    void OnPointerEnter() override;
    void OnPointerLeave() override;
    void OnPointerDown(float X, float Y) override;
    void OnPointerUp(bool StillHovered) override;
    void OnPointerDrag(float X, float Y, float DeltaX, float DeltaY) override;

    GameEvent<float> &OnValueChanged() { return ValueChanged; }

  private:
    void RefreshLabel();

    // Maps a screen X position across ComputedRect onto MinValue..MaxValue and applies it
    // (clamped, Step-snapped, like any other SetValue call). Only meaningful with HasRange set.
    void SetValueFromScreenX(float X);

    std::unique_ptr<Text> Label;

    float Value = 0.0f;
    float MinValue = 0.0f;
    float MaxValue = 0.0f;
    bool HasRange = false;
    float Step = 0.0f;
    float DragSensitivity = 0.1f;

    Size DesiredSize{80.0f, Theme::InputHeight};

    Color NormalColor = Theme::NeutralNormal;

    // Theme::PrimaryHovered/PrimaryPressed (blue family), not Theme::NeutralHovered/NeutralPressed
    // (a warm gray/brown that reads unrelated next to FillColor's blue below), same reasoning as
    // Dropdown's HoveredColor/PressedColor: hovering or pressing should feel like the same family
    // of feedback as the fill bar already showing, not a visually unrelated color.
    Color HoveredColor = Theme::PrimaryHovered;
    Color PressedColor = Theme::PrimaryPressed;

    // Theme::Accent (gold) reads as a warning here. A lighter blue (e.g. Palette::Slate) looks
    // fine against NormalColor but fails badly once the Label's white text sits on top of it
    // (1.6:1, both light colors); Theme::PrimaryNormal is a soft fill-vs-box edge (1.35:1) but
    // keeps the label readable everywhere the fill covers it (7.2:1), which matters more since
    // the label is the whole point of this widget.
    Color FillColor = Theme::PrimaryNormal;

    bool IsHovered = false;
    bool IsPressedDown = false;

    GameEvent<float> ValueChanged;
  };
}
