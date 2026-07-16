#pragma once

#include "Core.h"
#include "Widget.h"
#include <SDL_pixels.h>
#include <string>

struct _TTF_Font;
using TTF_Font = _TTF_Font;
struct SDL_Texture;

namespace Engine::UI
{
  // Renders a string with SDL_ttf. Measure only needs the font (TTF_SizeUTF8, no renderer
  // involved), so layout works without ever touching SDL_Renderer; the actual SDL_Texture is
  // lazily created (and re-created on change) the first time Render() runs, since that is the
  // only pass that has a renderer to create it with. Font/texture are both non-owning/owning as
  // appropriate: the TTF_Font* comes from AssetManager::LoadFont and outlives this widget, the
  // rendered SDL_Texture is this widget's own and gets destroyed on change or in the destructor.
  class ENGINE_API Text : public Widget
  {
  public:
    Text();
    ~Text() override;

    void SetFont(TTF_Font *NewFont);
    void SetText(const std::string &NewText);
    void SetColor(SDL_Color NewColor);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

  private:
    void RebuildTextureIfNeeded(SDL_Renderer *Renderer);

    TTF_Font *Font = nullptr;
    std::string Content;
    SDL_Color Color{255, 255, 255, 255};

    SDL_Texture *Texture = nullptr;
    bool Dirty = true;
  };
}
