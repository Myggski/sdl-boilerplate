#include "RenderSystem.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/SpriteComponent.h"
#include "src/Camera.h"

namespace Engine
{
  void RenderSystem(EntityManager &World, Camera &MainCamera)
  {
    World.ForEach<TransformComponent, SpriteComponent>(
        [&MainCamera](Entity, TransformComponent &Transform, SpriteComponent &Sprite)
        {
          if (!Sprite.TextureHandle)
          {
            return;
          }

          Rect DestRect{
              Transform.Position.X,
              Transform.Position.Y,
              Sprite.SourceRect.Width * Transform.Scale.X,
              Sprite.SourceRect.Height * Transform.Scale.Y};

          MainCamera.DrawSprite(Sprite.TextureHandle, Sprite.SourceRect, DestRect, Transform.Rotation);
        });
  }
}
