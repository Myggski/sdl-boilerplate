#include "SDLConversions.h"
#include <SDL_pixels.h>

namespace Engine::UI
{
  SDL_Color ToSDLColor(Color InColor)
  {
    return SDL_Color{InColor.R, InColor.G, InColor.B, InColor.A};
  }
}
