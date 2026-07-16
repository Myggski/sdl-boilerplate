#include "EngineContext.h"

namespace Engine
{
  EngineContext::EngineContext(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t Zoom)
      : Window(Window), Renderer(Renderer), Dispatcher(),
        Assets(Renderer), Input(Dispatcher), MainCamera(Window, Renderer, ScreenWidth, ScreenHeight, Zoom),
        Overlay(Window, Renderer, Dispatcher)
  {
  }
}
