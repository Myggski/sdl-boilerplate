#include "AnimationComponent.h"

namespace Engine
{
  AnimationComponent::AnimationComponent() = default;

  AnimationComponent::AnimationComponent(std::vector<Rect> Frames, float FrameTime, bool Loop)
      : Frames(std::move(Frames)), FrameTime(FrameTime), Loop(Loop) {}

  std::vector<Rect> MakeGridFrames(Vector2D Origin, float FrameWidth, float FrameHeight, size_t FrameCount)
  {
    std::vector<Rect> Frames;
    Frames.reserve(FrameCount);
    for (size_t Index = 0; Index < FrameCount; ++Index)
    {
      Frames.push_back(Rect{Origin.X + FrameWidth * static_cast<float>(Index), Origin.Y, FrameWidth, FrameHeight});
    }
    return Frames;
  }
}
