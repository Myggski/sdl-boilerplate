#include "Image.h"
#include <SDL3/SDL_render.h>

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

    float TextureWidth = 0.0f;
    float TextureHeight = 0.0f;
    SDL_GetTextureSize(TextureHandle, &TextureWidth, &TextureHeight);
    return Size{TextureWidth, TextureHeight};
  }

  void Image::Render(SDL_Renderer *Renderer)
  {
    if (!Visible || !TextureHandle)
    {
      return;
    }

    SDL_FRect DestRect{
        ComputedRect.X,
        ComputedRect.Y,
        ComputedRect.Width,
        ComputedRect.Height};

    if (SourceRect.Width > 0.0f && SourceRect.Height > 0.0f)
    {
      SDL_FRect SrcRect{
          SourceRect.X,
          SourceRect.Y,
          SourceRect.Width,
          SourceRect.Height};
      SDL_RenderTexture(Renderer, TextureHandle, &SrcRect, &DestRect);
    }
    else
    {
      SDL_RenderTexture(Renderer, TextureHandle, nullptr, &DestRect);
    }
  }
}
