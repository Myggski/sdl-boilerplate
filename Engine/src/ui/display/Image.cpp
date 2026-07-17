#include "Image.h"
#include <SDL_render.h>

namespace Engine::UI
{
  Image::Image() = default;
  Image::~Image() = default;

  Image *Image::SetTexture(Texture *NewTexture)
  {
    TextureHandle = reinterpret_cast<SDL_Texture *>(NewTexture);
    return this;
  }

  Image *Image::SetSourceRect(Rect NewSourceRect)
  {
    SourceRect = NewSourceRect;
    return this;
  }

  Image *Image::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
    return this;
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

    if (!TextureHandle)
    {
      return Size{};
    }

    int TextureWidth = 0;
    int TextureHeight = 0;
    SDL_QueryTexture(TextureHandle, nullptr, nullptr, &TextureWidth, &TextureHeight);
    return Size{static_cast<float>(TextureWidth), static_cast<float>(TextureHeight)};
  }

  void Image::Render(SDL_Renderer *Renderer)
  {
    if (!Visible || !TextureHandle)
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
      SDL_RenderCopy(Renderer, TextureHandle, &SrcRect, &DestRect);
    }
    else
    {
      SDL_RenderCopy(Renderer, TextureHandle, nullptr, &DestRect);
    }
  }
}
