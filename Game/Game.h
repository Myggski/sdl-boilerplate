#pragma once

struct SDL_Renderer;
struct SDL_Window;

namespace Game
{
  bool Initialize(SDL_Window *Window, SDL_Renderer *Renderer);
  void Update(float DeltaTime);
  void Draw(SDL_Renderer *Renderer);
  void Shutdown();

  SDL_Renderer *Renderer { nullptr };
}