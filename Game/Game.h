#pragma once

#include <memory>

struct SDL_Renderer;
struct SDL_Window;

namespace Engine
{
  class SDLEventDispatcher;
}

namespace Game
{
  int32_t SDLEventForImGuiHandle{-1};
  std::shared_ptr<Engine::SDLEventDispatcher> EngineSDLEventDispatcher{nullptr};

  bool Initialize(SDL_Window *Window, SDL_Renderer *Renderer, Engine::SDLEventDispatcher &SDLEventDispatcher);
  void Update(float DeltaTime);
  void Draw(SDL_Renderer *Renderer);
  void Shutdown();
};