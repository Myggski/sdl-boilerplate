#include "Checkbox.h"
#include <SDL_render.h>

namespace Engine::UI
{
  Checkbox::Checkbox()
  {
    BlocksInput = true;
  }

  Checkbox::~Checkbox() = default;

  void Checkbox::SetChecked(bool NewChecked)
  {
    Checked = NewChecked;
  }

  void Checkbox::SetBoxColors(SDL_Color Normal, SDL_Color Hovered, SDL_Color Pressed)
  {
    NormalColor = Normal;
    HoveredColor = Hovered;
    PressedColor = Pressed;
  }

  void Checkbox::SetCheckColor(SDL_Color NewColor)
  {
    CheckColor = NewColor;
  }

  void Checkbox::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
  }

  Size Checkbox::Measure(Size)
  {
    return DesiredSize;
  }

  void Checkbox::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    SDL_Color BoxColor = IsPressedDown ? PressedColor : (IsHovered ? HoveredColor : NormalColor);

    SDL_Rect BoxRect{
        static_cast<int>(ComputedRect.X),
        static_cast<int>(ComputedRect.Y),
        static_cast<int>(ComputedRect.Width),
        static_cast<int>(ComputedRect.Height)};

    SDL_BlendMode PreviousBlendMode;
    SDL_GetRenderDrawBlendMode(Renderer, &PreviousBlendMode);
    SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(Renderer, BoxColor.r, BoxColor.g, BoxColor.b, BoxColor.a);
    SDL_RenderFillRect(Renderer, &BoxRect);

    if (Checked)
    {
      float InsetX = ComputedRect.Width * 0.25f;
      float InsetY = ComputedRect.Height * 0.25f;
      SDL_Rect CheckRect{
          static_cast<int>(ComputedRect.X + InsetX),
          static_cast<int>(ComputedRect.Y + InsetY),
          static_cast<int>(ComputedRect.Width - InsetX * 2.0f),
          static_cast<int>(ComputedRect.Height - InsetY * 2.0f)};

      SDL_SetRenderDrawColor(Renderer, CheckColor.r, CheckColor.g, CheckColor.b, CheckColor.a);
      SDL_RenderFillRect(Renderer, &CheckRect);
    }

    SDL_SetRenderDrawBlendMode(Renderer, PreviousBlendMode);
  }

  void Checkbox::OnPointerEnter()
  {
    IsHovered = true;
  }

  void Checkbox::OnPointerLeave()
  {
    IsHovered = false;
    IsPressedDown = false;
  }

  void Checkbox::OnPointerDown()
  {
    IsPressedDown = true;
  }

  void Checkbox::OnPointerUp(bool StillHovered)
  {
    IsPressedDown = false;

    if (StillHovered)
    {
      Checked = !Checked;
      CheckedChanged.Broadcast(Checked);
    }
  }
}
