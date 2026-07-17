#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"
#include "GameEvent.h"

namespace Engine::UI
{
  // A small toggleable box. Click (press then release while still hovered, same semantics as
  // Button) flips Checked and broadcasts OnCheckedChanged. Renders as a box (hover/press colors,
  // same feedback as Button) with a smaller filled inner square when checked, no glyph/checkmark
  // rendering.
  class ENGINE_API Checkbox : public Widget
  {
  public:
    Checkbox();
    ~Checkbox() override;

    Checkbox *SetChecked(bool NewChecked);
    bool IsChecked() const { return Checked; }

    Checkbox *SetBoxColors(Color Normal, Color Hovered, Color Pressed);
    Checkbox *SetCheckColor(Color NewColor);
    Checkbox *SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

    void OnPointerEnter() override;
    void OnPointerLeave() override;
    void OnPointerDown(float X, float Y) override;
    void OnPointerUp(bool StillHovered) override;

    GameEvent<bool> &OnCheckedChanged() { return CheckedChanged; }

  private:
    // Grid-aligned (Theme::Spacing::Large), smaller than Theme::MinTouchTarget: a checkbox
    // conventionally pairs with a clickable label for the full touch target in most UIs, same as
    // this widget's own OnClicked-style semantics let a caller wrap it in a bigger clickable area
    // if needed.
    Size DesiredSize{Theme::Spacing::Large, Theme::Spacing::Large};

    Color NormalColor = Theme::NeutralNormal;
    Color HoveredColor = Theme::NeutralHovered;
    Color PressedColor = Theme::NeutralPressed;

    // Same color as Scalar's fill (Theme::PrimaryNormal), so the two small "how much/whether"
    // indicators in a toolbar read as one consistent accent rather than two different ones.
    Color CheckColor = Theme::PrimaryNormal;

    bool Checked = false;
    bool IsHovered = false;
    bool IsPressedDown = false;

    GameEvent<bool> CheckedChanged;
  };
}
