#include "Image.h"
#include <SDL_render.h>

namespace Engine::UI
{
  Image::Image() = default;
  Image::~Image() = default;

  void Image::SetTexture(SDL_Texture *NewTexture)
  {
    Texture = NewTexture;
  }

  void Image::SetSourceRect(Rect NewSourceRect)
  {
    SourceRect = NewSourceRect;
  }

  void Image::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
  }

  Size Image::Measure(Size)
  {
    if (DesiredSize.Width > 0.0f || DesiredSize.Height > 0.0f)
    {
      return DesiredSize;
    }

    if (SourceRect.Width > 0.0f && SourceRect.Height > 0.0f)
    {
      return Size{SourceRect.Width, SourceRect.Height};
    }

    if (!Texture)
    {
      return Size{};
    }

    int TextureWidth = 0;
    int TextureHeight = 0;
    SDL_QueryTexture(Texture, nullptr, nullptr, &TextureWidth, &TextureHeight);
    return Size{static_cast<float>(TextureWidth), static_cast<float>(TextureHeight)};
  }

  void Image::Render(SDL_Renderer *Renderer)
  {
    if (!Visible || !Texture)
    {
      return;
    }

    SDL_Rect DestRect{
        static_cast<int>(ComputedRect.X),
        static_cast<int>(ComputedRect.Y),
        static_cast<int>(ComputedRect.Width),
        static_cast<int>(ComputedRect.Height)};

    if (SourceRect.Width > 0.0f && SourceRect.Height > 0.0f)
    {
      SDL_Rect SrcRect{
          static_cast<int>(SourceRect.X),
          static_cast<int>(SourceRect.Y),
          static_cast<int>(SourceRect.Width),
          static_cast<int>(SourceRect.Height)};
      SDL_RenderCopy(Renderer, Texture, &SrcRect, &DestRect);
    }
    else
    {
      SDL_RenderCopy(Renderer, Texture, nullptr, &DestRect);
    }
  }
}
