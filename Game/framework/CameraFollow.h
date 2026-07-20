#pragma once

#include "src/EngineContext.h"

namespace Game
{
  // Keeps every current player fit on screen at once, zooming out as they spread apart. Called
  // once per frame from Game::PostUpdate. No-op if no players exist yet.
  void UpdateCameraFollow(Engine::EngineContext &Context, float DeltaTime);
}
