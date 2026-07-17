#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"
#include "../../AssetManager.h"
#include <string>
#include <memory>

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

    Text *SetFont(TTF_Font *NewFont);
    Text *SetText(const std::string &NewText);
    Text *SetColor(Color NewColor);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

  private:
    void RebuildTextureIfNeeded(SDL_Renderer *Renderer);

    TTF_Font *Font = nullptr;
    std::string Content;
    Color TextColor = Theme::TextPrimary;

    SDL_Texture *Texture = nullptr;
    bool Dirty = true;
  };

  // Convenience: builds a Text widget with a loaded font in one call, rather than the multi-step
  // CreateWidget<Text>()->SetFont(Assets.LoadDefaultFont(Size))->SetText(...)->SetColor(...)
  // dance. Style defaults come from Theme::TextStyle; pass e.g. {.PointSize = 20} to override
  // just that field. Takes AssetManager explicitly (not a global default font) so the dependency
  // stays visible at the call site and there is no reliance on engine startup having run first.
  inline std::unique_ptr<Text> CreateLabel(Engine::AssetManager &Assets, const std::string &InText, Theme::TextStyle Style = {})
  {
    std::unique_ptr<Text> Label = CreateWidget<Text>();
    Label->SetFont(Assets.LoadDefaultFont(Style.PointSize))->SetText(InText)->SetColor(Style.Color);
    return Label;
  }
}
