#pragma once

#include "Core.h"
#include "Widget.h"

struct SDL_Texture;

namespace Engine::UI
{
  // Renders a texture stretched into whatever rect Arrange gives it. Non-owning, same convention
  // as Game.cpp's own SDL_Texture* fields: the texture comes from AssetManager::LoadTexture and
  // outlives this widget, Image never loads or frees it.
  class ENGINE_API Image : public Widget
  {
  public:
    Image();
    ~Image() override;

    void SetTexture(SDL_Texture *NewTexture);

    // Optional: crops to a sub-rect of the texture's own pixels (e.g. one icon out of a sprite
    // sheet) instead of using the whole thing. {0,0,0,0} (the default) means "whole texture".
    void SetSourceRect(Rect NewSourceRect);

    // Optional: overrides Measure's result. Without one, Measure reports SourceRect's size if
    // set, otherwise the texture's own pixel size (via SDL_QueryTexture), same "{0,0} means no
    // intrinsic size" convention Panel and Button already use for their own DesiredSize.
    void SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

  private:
    SDL_Texture *Texture = nullptr;
    Rect SourceRect{};
    Size DesiredSize{};
  };
}
