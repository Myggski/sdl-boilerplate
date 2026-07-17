#pragma once

#include "Core.h"

namespace Engine
{
  // World/screen-space rect (pixels, not SDL_Rect): game code builds one of these to describe a
  // sprite's source/dest, Camera::DrawSprite converts to SDL_Rect internally, the same
  // convert-deep-in-engine pattern as Engine::UI::Color/ToSDLColor.
  struct ENGINE_API Rect
  {
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
  };
}
