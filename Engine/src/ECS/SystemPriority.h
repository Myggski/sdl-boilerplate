#pragma once

#include <cstdint>

namespace Engine::SystemPriority
{
  // Priorities EngineContext auto-registers Movement/Animation/Collision at (see
  // EngineContext.cpp). Every game needs these, so they run without a game having to remember
  // to register each one itself. Lower runs first. A game registering its own system should pick
  // a priority relative to these constants (e.g. Movement - 1 to run before movement, 250 to land
  // between Animation and Collision) rather than a guessed number.
  constexpr int32_t Movement = 100;
  constexpr int32_t Animation = 200;
  constexpr int32_t Collision = 300;
}
