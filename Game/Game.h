#pragma once

#include <memory>

struct SDL_Renderer;
struct SDL_Window;

namespace Game
{
  int32_t SDLEventForImGuiHandle{-1};

  bool Initialize(SDL_Window *Window, SDL_Renderer *Renderer);
  void Update(float DeltaTime);
  void Draw(SDL_Renderer *Renderer);
  void Shutdown();
};