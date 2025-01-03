#pragma once

#include "Core.h"

namespace Engine
{
  struct ENGINE_API VelocityComponent
  {
  public:
    VelocityComponent();
    VelocityComponent(float X, float Y);

  public:
    float X = 0.0f;
    float Y = 0.0f;
  };
}
