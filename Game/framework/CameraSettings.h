#pragma once

namespace Game
{
  namespace CameraTuning
  {
    constexpr float MaxZoom = 1.0f;
    constexpr float MinZoom = 0.5f;
    constexpr float Padding = 24.0f;      // world units kept clear around players at tightest zoom
    constexpr float FollowSmoothing = 8.0f; // higher = snappier camera pan/zoom
  }
}
