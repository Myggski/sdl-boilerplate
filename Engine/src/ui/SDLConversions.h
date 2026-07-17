#pragma once

#include "Widget.h"

struct SDL_Color;

namespace Engine::UI
{
  // Conversions from Engine's own UI types to their real SDL equivalents, for the few call sites
  // deep in the engine that actually need one (e.g. SDL_ttf's TTF_RenderUTF8_Blended, see
  // Text.cpp, needs a real SDL_Color). Deliberately not included from Engine.h: game code only
  // ever sees Engine's own types (Color, Rect, ...) through the normal #include "Engine.h"
  // umbrella, never the SDL types themselves. Only Engine's own .cpp files reach this by including
  // it directly. Add more ToSDL*() overloads here as more of these conversions come up.
  SDL_Color ToSDLColor(Color InColor);
}
