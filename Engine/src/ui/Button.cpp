#include "Button.h"
#include <SDL_render.h>

namespace Engine::UI
{
  Button::Button()
  {
    BlocksInput = true;
  }

  Button::~Button() = default;

  Widget *Button::SetContent(std::unique_ptr<Widget> NewContent)
  {
    Widget *Result = NewContent.get();
    Content = std::move(NewContent);
    return Result;
  }

  void Button::SetColors(SDL_Color Normal, SDL_Color Hovered, SDL_Color Pressed)
  {
    NormalColor = Normal;
    HoveredColor = Hovered;
    PressedColor = Pressed;
  }

  void Button::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
  }

  void Button::SetBackgroundImage(SDL_Texture *Texture)
  {
    BackgroundImage = Texture;
  }

  void Button::SetBackgroundImageSourceRect(Rect NewSourceRect)
  {
    BackgroundImageSourceRect = NewSourceRect;
  }

  Size Button::Measure(Size AvailableSize)
  {
    if (Content)
    {
      return Content->Measure(AvailableSize);
    }
    return DesiredSize;
  }

  void Button::Arrange(Rect FinalRect)
  {
    ComputedRect = FinalRect;
    if (Content)
    {
      Content->Arrange(FinalRect);
    }
  }

  void Button::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    SDL_Rect DestRect{
        static_cast<int>(ComputedRect.X),
        static_cast<int>(ComputedRect.Y),
        static_cast<int>(ComputedRect.Width),
        static_cast<int>(ComputedRect.Height)};

    if (BackgroundImage)
    {
      Uint8 Tint = IsPressedDown ? 180 : 255;

      Uint8 PreviousR, PreviousG, PreviousB;
      SDL_GetTextureColorMod(BackgroundImage, &PreviousR, &PreviousG, &PreviousB);
      SDL_SetTextureColorMod(BackgroundImage, Tint, Tint, Tint);

      if (BackgroundImageSourceRect.Width > 0.0f && BackgroundImageSourceRect.Height > 0.0f)
      {
        SDL_Rect SrcRect{
            static_cast<int>(BackgroundImageSourceRect.X),
            static_cast<int>(BackgroundImageSourceRect.Y),
            static_cast<int>(BackgroundImageSourceRect.Width),
            static_cast<int>(BackgroundImageSourceRect.Height)};
        SDL_RenderCopy(Renderer, BackgroundImage, &SrcRect, &DestRect);
      }
      else
      {
        SDL_RenderCopy(Renderer, BackgroundImage, nullptr, &DestRect);
      }

      SDL_SetTextureColorMod(BackgroundImage, PreviousR, PreviousG, PreviousB);
    }
    else
    {
      SDL_Color Current = IsPressedDown ? PressedColor : (IsHovered ? HoveredColor : NormalColor);

      SDL_BlendMode PreviousBlendMode;
      SDL_GetRenderDrawBlendMode(Renderer, &PreviousBlendMode);
      SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

      SDL_SetRenderDrawColor(Renderer, Current.r, Current.g, Current.b, Current.a);
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

  void Button::OnPointerDown()
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
