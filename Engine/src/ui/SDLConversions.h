#pragma once

#include "Widget.h"

struct SDL_Color;

namespace Engine::UI
{
  // Conversions from Engine's own UI types to real SDL equivalents, for the few call sites that
  // need one (e.g. Text.cpp's TTF_RenderUTF8_Blended). Not included from Engine.h - game code
  // never sees SDL types directly.
  SDL_Color ToSDLColor(Color InColor);
}
