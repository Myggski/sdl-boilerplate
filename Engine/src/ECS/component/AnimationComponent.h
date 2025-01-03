#pragma once

#include "Core.h"
#include <vector>
#include <SDL.h>

namespace Engine
{
  struct ENGINE_API AnimationComponent
  {
  public:
    AnimationComponent() = default;
    AnimationComponent(const std::vector<SDL_Rect> &frames, float frameTime, bool loop = true)
        : Frames(std::move(frames)), FrameTime(frameTime), Loop(loop) {}

  public:
    float FrameTime = 0.1f;       // Time to display each frame (in seconds)
    float Timer = 0.0f;           // Tracks time elapsed since the last frame change
    size_t CurrentFrame = 0;      // Index of the current frame
    std::vector<SDL_Rect> Frames; // Frames of the animation
    bool Loop = true;             // Should the animation loop?
  };
}