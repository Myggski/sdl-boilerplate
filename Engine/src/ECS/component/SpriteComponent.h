#pragma once

#include "Core.h"
#include "Texture.h"
#include "src/math/Rect.h"

namespace Engine
{
  // World-space sprite draw data for RenderSystem: a texture plus the sub-rect of it to draw
  // (SourceRect), e.g. one frame of a spritesheet, or one region of a larger shared "master
  // image"/atlas (AssetManager::LoadTexture already caches by file path, so many entities
  // pointing at the same file share one Texture*). No DestRect: world draw size is SourceRect's
  // own width/height scaled by TransformComponent::Scale, reusing Scale as the one existing
  // scale-multiplier rather than a second size concept.
  struct ENGINE_API SpriteComponent
  {
  public:
    SpriteComponent();
    SpriteComponent(Texture *TextureHandle, Rect SourceRect);

  public:
    Texture *TextureHandle = nullptr;
    Rect SourceRect{};
  };
}
