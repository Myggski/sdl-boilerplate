#include "Button.h"
#include <SDL3/SDL_render.h>

namespace Engine::UI
{
  Button::Button()
  {
    BlocksInput = true;
  }

  Button::~Button() = default;

  Button *Button::SetColors(Color Normal, Color Hovered, Color Pressed)
  {
    NormalColor = Normal;
    HoveredColor = Hovered;
    PressedColor = Pressed;
    return this;
  }

  Button *Button::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
    return this;
  }

  Button *Button::SetBackgroundImage(Texture *NewTexture)
  {
    BackgroundImage = reinterpret_cast<SDL_Texture *>(NewTexture);
    return this;
  }

  Button *Button::SetBackgroundImageSourceRect(Rect NewSourceRect)
  {
    BackgroundImageSourceRect = NewSourceRect;
    return this;
  }

  Button *Button::SetContentPadding(Padding NewPadding)
  {
    ContentPadding = NewPadding;
    return this;
  }

  Size Button::Measure(Size AvailableSize)
  {
    if (Content)
    {
      Size ContentSize = Content->Measure(AvailableSize);
      return Size{
          ContentSize.Width + ContentPadding.Left + ContentPadding.Right,
          ContentSize.Height + ContentPadding.Top + ContentPadding.Bottom};
    }
    return DesiredSize;
  }

  void Button::Arrange(Rect FinalRect)
  {
    ComputedRect = FinalRect;
    if (Content)
    {
      Rect InsetRect{
          FinalRect.X + ContentPadding.Left,
          FinalRect.Y + ContentPadding.Top,
          FinalRect.Width - ContentPadding.Left - ContentPadding.Right,
          FinalRect.Height - ContentPadding.Top - ContentPadding.Bottom};
      Content->Arrange(InsetRect);
    }
  }

  void Button::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    SDL_FRect DestRect{
        ComputedRect.X,
        ComputedRect.Y,
        ComputedRect.Width,
        ComputedRect.Height};

    if (BackgroundImage)
    {
      Uint8 Tint = IsPressedDown ? 180 : 255;

      Uint8 PreviousR, PreviousG, PreviousB;
      SDL_GetTextureColorMod(BackgroundImage, &PreviousR, &PreviousG, &PreviousB);
      SDL_SetTextureColorMod(BackgroundImage, Tint, Tint, Tint);

      if (BackgroundImageSourceRect.Width > 0.0f && BackgroundImageSourceRect.Height > 0.0f)
      {
        SDL_FRect SrcRect{
            BackgroundImageSourceRect.X,
            BackgroundImageSourceRect.Y,
            BackgroundImageSourceRect.Width,
            BackgroundImageSourceRect.Height};
        SDL_RenderTexture(Renderer, BackgroundImage, &SrcRect, &DestRect);
      }
      else
      {
        SDL_RenderTexture(Renderer, BackgroundImage, nullptr, &DestRect);
      }

      SDL_SetTextureColorMod(BackgroundImage, PreviousR, PreviousG, PreviousB);
    }
    else
    {
      Color Current = IsPressedDown ? PressedColor : (IsHovered ? HoveredColor : NormalColor);

      SDL_BlendMode PreviousBlendMode;
      SDL_GetRenderDrawBlendMode(Renderer, &PreviousBlendMode);
      SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

      SDL_SetRenderDrawColor(Renderer, Current.R, Current.G, Current.B, Current.A);
      SDL_RenderFillRect(Renderer, &DestRect);

      SDL_SetRenderDrawBlendMode(Renderer, PreviousBlendMode);
    }

    if (Content)
    {
      Content->Render(Renderer);
    }
  }

  void Button::OnPointerEnter()
  {
    IsHovered = true;
  }

  void Button::OnPointerLeave()
  {
    IsHovered = false;
    IsPressedDown = false;
  }

  void Button::OnPointerDown(float, float)
  {
    IsPressedDown = true;
    Pressed.Broadcast();
  }

  void Button::OnPointerUp(bool StillHovered)
  {
    IsPressedDown = false;
    Released.Broadcast();

    if (StillHovered)
    {
      Clicked.Broadcast();
    }
  }
}
