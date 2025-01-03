#pragma once

#include "Core.h"
#include <memory>
#include <SDL.h>

namespace Engine
{
  struct ENGINE_API SpriteComponent
  {
    SDL_Texture *Texture = nullptr;
    SDL_Rect SourceRect{0, 0, 0, 0};
    SDL_Rect DestRect{0, 0, 0, 0};

    SpriteComponent() = default;

    SpriteComponent(SDL_Texture *Texture, const SDL_Rect &SourceRect, const SDL_Rect &DestRect)
        : Texture(Texture), SourceRect(SourceRect), DestRect(DestRect) {}
  };
}
