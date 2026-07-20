#pragma once

#include <cstdint>

namespace Engine::SystemPriority
{
  // Priorities EngineContext auto-registers its built-in systems at (see EngineContext.cpp).
  // Lower runs first. Pick a priority relative to these (e.g. Movement - 1, or 250 to land between
  // Animation and Collision) rather than a guessed number.
  // Pathfinding/FlowFieldPathfinding are separate constants (not one shared value) purely so
  // RegisterSystem's reused-priority warning stays meaningful elsewhere; no entity carries both.
  constexpr int32_t Pathfinding = 50;
  constexpr int32_t FlowFieldPathfinding = 51;
  constexpr int32_t Movement = 100;
  constexpr int32_t Animation = 200;
  constexpr int32_t Collision = 300;
}
