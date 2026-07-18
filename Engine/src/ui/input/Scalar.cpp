#include "Scalar.h"
#include "../display/Text.h"
#include "../../AssetManager.h"
#include <SDL_render.h>
#include <cstdio>
#include <cmath>

namespace Engine::UI
{
  Scalar::Scalar()
  {
    BlocksInput = true;
    Label = std::make_unique<Text>();
    Label->SetColor(Theme::TextPrimary);
    RefreshLabel();
  }

  Scalar::~Scalar() = default;

  Scalar *Scalar::SetFont(TTF_Font *NewFont)
  {
    Label->SetFont(NewFont);
    return this;
  }

  Scalar *Scalar::SetFont(Engine::AssetManager &Assets, Theme::TextStyle Style)
  {
    SetFont(Assets.LoadDefaultFont(Style.PointSize));
    return SetTextColor(Style.Color);
  }

  Scalar *Scalar::SetValue(float NewValue)
  {
    Value = NewValue;

    if (Step > 0.0f)
    {
      Value = std::round(Value / Step) * Step;
    }

    if (HasRange)
    {
      Value = Value < MinValue ? MinValue : (Value > MaxValue ? MaxValue : Value);
    }

    // std::round can land exactly on IEEE 754 negative zero for small negative inputs (e.g.
    // std::round(-0.3f) == -0.0f), which "%.0f" then faithfully prints as "-0". Value == 0.0f is
    // true for both signs, so this reassignment quietly normalizes -0.0f to +0.0f without
    // affecting any other value.
    if (Value == 0.0f)
    {
      Value = 0.0f;
    }

    RefreshLabel();
    return this;
  }

  Scalar *Scalar::SetRange(float NewMin, float NewMax)
  {
    HasRange = NewMin <= NewMax;
    MinValue = NewMin;
    MaxValue = NewMax;
    if (HasRange)
    {
      SetValue(Value);
    }
    return this;
  }

  Scalar *Scalar::SetStep(float NewStep)
  {
    Step = NewStep;
    SetValue(Value);
    return this;
  }

  Scalar *Scalar::SetDragSensitivity(float NewSensitivity)
  {
    DragSensitivity = NewSensitivity;
    return this;
  }

  Scalar *Scalar::SetColors(Color Normal, Color Hovered, Color Pressed)
  {
    NormalColor = Normal;
    HoveredColor = Hovered;
    PressedColor = Pressed;
    return this;
  }

  Scalar *Scalar::SetFillColor(Color NewColor)
  {
    FillColor = NewColor;
    return this;
  }

  Scalar *Scalar::SetTextColor(Color NewColor)
  {
    Label->SetColor(NewColor);
    return this;
  }

  Scalar *Scalar::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
    return this;
  }

  void Scalar::RefreshLabel()
  {
    char Buffer[32];

    // Whole-number Step (1, 10, ...) reads better without decimals; a fractional Step (0.1, ...)
    // or no Step at all (continuous) keeps one decimal place.
    if (Step > 0.0f && std::fmod(Step, 1.0f) == 0.0f)
    {
      std::snprintf(Buffer, sizeof(Buffer), "%.0f", Value);
    }
    else
    {
      std::snprintf(Buffer, sizeof(Buffer), "%.1f", Value);
    }

    Label->SetText(Buffer);
  }

  Size Scalar::Measure(Size)
  {
    return DesiredSize;
  }

  void Scalar::Arrange(Rect FinalRect)
  {
    ComputedRect = FinalRect;
    Label->Arrange(FinalRect);
  }

  void Scalar::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    Color Current = IsPressedDown ? PressedColor : (IsHovered ? HoveredColor : NormalColor);

    SDL_Rect DestRect{
        static_cast<int>(ComputedRect.X),
        static_cast<int>(ComputedRect.Y),
        static_cast<int>(ComputedRect.Width),
        static_cast<int>(ComputedRect.Height)};

    SDL_BlendMode PreviousBlendMode;
    SDL_GetRenderDrawBlendMode(Renderer, &PreviousBlendMode);
    SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(Renderer, Current.R, Current.G, Current.B, Current.A);
    SDL_RenderFillRect(Renderer, &DestRect);

    if (HasRange && MaxValue > MinValue)
    {
      float Fraction = (Value - MinValue) / (MaxValue - MinValue);
      Fraction = Fraction < 0.0f ? 0.0f : (Fraction > 1.0f ? 1.0f : Fraction);

      SDL_Rect FillRect{
          DestRect.x,
          DestRect.y,
          static_cast<int>(DestRect.w * Fraction),
          DestRect.h};

      SDL_SetRenderDrawColor(Renderer, FillColor.R, FillColor.G, FillColor.B, FillColor.A);
      SDL_RenderFillRect(Renderer, &FillRect);
    }

    SDL_SetRenderDrawBlendMode(Renderer, PreviousBlendMode);

    Label->Render(Renderer);
  }

  void Scalar::OnPointerEnter()
  {
    IsHovered = true;
  }

  void Scalar::OnPointerLeave()
  {
    // Not IsPressedDown = false here: Canvas keeps forwarding OnPointerDrag to this widget by
    // capture (PressedWidget) even while the pointer is outside its bounds, so a drag started
    // inside should keep reading as pressed (and keep affecting Value) until the actual
    // OnPointerUp, not revert to hovered/normal just because the pointer left mid-drag.
    IsHovered = false;
  }

  void Scalar::SetValueFromScreenX(float X)
  {
    if (!HasRange || ComputedRect.Width <= 0.0f)
    {
      return;
    }

    float Fraction = (X - ComputedRect.X) / ComputedRect.Width;
    Fraction = Fraction < 0.0f ? 0.0f : (Fraction > 1.0f ? 1.0f : Fraction);
    SetValue(MinValue + Fraction * (MaxValue - MinValue));
    ValueChanged.Broadcast(Value);
  }

  void Scalar::OnPointerDown(float X, float)
  {
    IsPressedDown = true;
    SetValueFromScreenX(X);
  }

  void Scalar::OnPointerUp(bool)
  {
    IsPressedDown = false;
  }

  void Scalar::OnPointerDrag(float X, float, float DeltaX, float)
  {
    if (HasRange)
    {
      // Same absolute position-to-value mapping as OnPointerDown, so drag always matches
      // exactly where the pointer is instead of drifting from a separately-tuned sensitivity.
      SetValueFromScreenX(X);
    }
    else
    {
      SetValue(Value + DeltaX * DragSensitivity);
      ValueChanged.Broadcast(Value);
    }
  }
}
