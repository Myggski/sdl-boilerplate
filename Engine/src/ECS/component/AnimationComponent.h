#pragma once

#include "Core.h"
#include "src/math/Rect.h"
#include "src/math/Vector2D.h"
#include <cstddef>
#include <vector>

namespace Engine
{
  // Paired with a SpriteComponent on the same entity: AnimationSystem steps through Frames over
  // time and writes the current one into that SpriteComponent's SourceRect. An entity with only a
  // SpriteComponent (no AnimationComponent) is simply never touched by AnimationSystem's
  // ForEach<AnimationComponent, SpriteComponent> query, and stays static — no special-casing
  // needed on either side for static-vs-animated to coexist.
  struct ENGINE_API AnimationComponent
  {
  public:
    AnimationComponent();
    AnimationComponent(std::vector<Rect> Frames, float FrameTime, bool Loop = true);

  public:
    std::vector<Rect> Frames; // Frames of the animation, in playback order
    float FrameTime = 0.1f;   // Time to display each frame (in seconds)
    float Timer = 0.0f;       // Tracks time elapsed since the last frame change
    size_t CurrentFrame = 0;  // Index of the current frame
    bool Loop = true;         // Loop back to frame 0, or hold on the last frame, when done?
    bool Playing = true;      // AnimationSystem only advances Timer/CurrentFrame while true
  };

  // Builds a row of FrameCount equal-size frames from a spritesheet, left to right starting at
  // Origin, for AnimationComponent::Frames — e.g. MakeGridFrames({0, 0}, 16.0f, 16.0f, 4) for
  // bomb.png's 4 horizontal 16x16 frames, instead of hand-writing 4 Rect literals. Single-row
  // only for now; extend if/when a game asset actually needs a multi-row grid.
  ENGINE_API std::vector<Rect> MakeGridFrames(Vector2D Origin, float FrameWidth, float FrameHeight, size_t FrameCount);
}
