#include "AnimationSystem.h"
#include "src/ecs/EntityManager.h"
#include "src/ecs/component/AnimationComponent.h"
#include "src/ecs/component/SpriteComponent.h"

namespace Engine
{
  void AnimationSystem(EntityManager &World, float DeltaTime)
  {
    World.ForEach<AnimationComponent, SpriteComponent>(
        [DeltaTime](Entity, AnimationComponent &Anim, SpriteComponent &Sprite)
        {
          if (!Anim.Playing || Anim.Frames.empty())
          {
            return;
          }

          Anim.Timer += DeltaTime;
          if (Anim.Timer >= Anim.FrameTime)
          {
            Anim.Timer -= Anim.FrameTime;
            ++Anim.CurrentFrame;
            if (Anim.CurrentFrame >= Anim.Frames.size())
            {
              Anim.CurrentFrame = Anim.Loop ? 0 : Anim.Frames.size() - 1;
              Anim.Playing = Anim.Loop;
            }
          }

          Sprite.SourceRect = Anim.Frames[Anim.CurrentFrame];
        });
  }
}
