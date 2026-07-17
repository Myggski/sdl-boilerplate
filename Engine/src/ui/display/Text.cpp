#include "Text.h"
#include "../SDLConversions.h"
#include <SDL_ttf.h>
#include <SDL_render.h>

namespace Engine::UI
{
  Text::Text() = default;

  Text::~Text()
  {
    if (Texture)
    {
      SDL_DestroyTexture(Texture);
    }
  }

  Text *Text::SetFont(TTF_Font *NewFont)
  {
    Font = NewFont;
    Dirty = true;
    return this;
  }

  Text *Text::SetText(const std::string &NewText)
  {
    Content = NewText;
    Dirty = true;
    return this;
  }

  Text *Text::SetColor(Color NewColor)
  {
    TextColor = NewColor;
    Dirty = true;
    return this;
  }

  Size Text::Measure(Size)
  {
    if (!Font || Content.empty())
    {
      return Size{};
    }

    int Width = 0;
    int Height = 0;
    TTF_SizeUTF8(Font, Content.c_str(), &Width, &Height);
    return Size{static_cast<float>(Width), static_cast<float>(Height)};
  }

  void Text::RebuildTextureIfNeeded(SDL_Renderer *Renderer)
  {
    if (!Dirty)
    {
      return;
    }

    if (Texture)
    {
      SDL_DestroyTexture(Texture);
      Texture = nullptr;
    }

    if (Font && !Content.empty())
    {
      SDL_Surface *Surface = TTF_RenderUTF8_Blended(Font, Content.c_str(), ToSDLColor(TextColor));
      if (Surface)
      {
        Texture = SDL_CreateTextureFromSurface(Renderer, Surface);
        SDL_FreeSurface(Surface);
      }
    }

    Dirty = false;
  }

  void Text::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    RebuildTextureIfNeeded(Renderer);

    if (!Texture)
    {
      return;
    }

    int TextureWidth = 0;
    int TextureHeight = 0;
    SDL_QueryTexture(Texture, nullptr, nullptr, &TextureWidth, &TextureHeight);

    // Centered within whatever space Arrange gave this widget, rather than stretched, text keeps
    // its natural glyph size regardless of how much room a Fill/cross-axis slot handed it.
    SDL_Rect DestRect{
        static_cast<int>(ComputedRect.X + (ComputedRect.Width - TextureWidth) * 0.5f),
        static_cast<int>(ComputedRect.Y + (ComputedRect.Height - TextureHeight) * 0.5f),
        TextureWidth,
        TextureHeight};

    SDL_RenderCopy(Renderer, Texture, nullptr, &DestRect);
  }
}
