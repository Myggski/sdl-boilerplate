#pragma once

#include "Core.h"

namespace Engine
{
  class EntityManager;
  class Camera;

  // Draws every entity with a Transform + Sprite via MainCamera.DrawSprite. Not registered
  // through EntityManager::RegisterSystem (that only gets a DeltaTime, no Camera, and runs 0-5x
  // per rendered frame under the fixed-timestep loop, which would double/triple-draw); call this
  // once per frame from Game::Draw instead.
  ENGINE_API void RenderSystem(EntityManager &World, Camera &MainCamera);
}
