#pragma once

#include "Core.h"

namespace Engine
{
  class EntityManager;

  // Advances AnimationComponent::Timer/CurrentFrame for every entity with both an Animation and a
  // Sprite, writing the current frame's Rect into that SpriteComponent's SourceRect. Opt-in like
  // any other system, register it yourself, e.g.
  //   Context.World.RegisterSystem([&Context](float DeltaTime)
  //     { AnimationSystem(Context.World, DeltaTime); });
  ENGINE_API void AnimationSystem(EntityManager &World, float DeltaTime);
}
