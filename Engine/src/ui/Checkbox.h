#pragma once

#include "Core.h"
#include "Widget.h"
#include "GameEvent.h"
#include <SDL_pixels.h>

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

    void SetChecked(bool NewChecked);
    bool IsChecked() const { return Checked; }

    void SetBoxColors(SDL_Color Normal, SDL_Color Hovered, SDL_Color Pressed);
    void SetCheckColor(SDL_Color NewColor);
    void SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

    void OnPointerEnter() override;
    void OnPointerLeave() override;
    void OnPointerDown() override;
    void OnPointerUp(bool StillHovered) override;

    GameEvent<bool> &OnCheckedChanged() { return CheckedChanged; }

  private:
    Size DesiredSize{20.0f, 20.0f};

    SDL_Color NormalColor{60, 60, 65, 255};
    SDL_Color HoveredColor{85, 85, 92, 255};
    SDL_Color PressedColor{40, 40, 45, 255};
    SDL_Color CheckColor{230, 200, 60, 255};

    bool Checked = false;
    bool IsHovered = false;
    bool IsPressedDown = false;

    GameEvent<bool> CheckedChanged;
  };
}
