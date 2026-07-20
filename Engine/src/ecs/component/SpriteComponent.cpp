#include "SpriteComponent.h"

namespace Engine
{
  SpriteComponent::SpriteComponent() = default;

  SpriteComponent::SpriteComponent(Texture *TextureHandle, Rect SourceRect)
      : TextureHandle(TextureHandle), SourceRect(SourceRect) {}
}
