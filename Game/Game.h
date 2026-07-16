#pragma once

namespace Engine
{
  struct EngineContext;
}

// The four callbacks that make a game. Startup and Shutdown are the bookends: Startup runs once
// at the beginning (register systems, create entities, load assets), Shutdown runs once at the
// end (undo/release whatever Startup set up; leave it empty if there's nothing to clean up).
// Update runs on a fixed timestep, after any systems registered in Startup already ran; put extra
// per-frame game logic here. Draw runs once per rendered frame; Dear ImGui is already set up, so
// ImGui:: calls just work.
namespace Game
{
  bool Startup(Engine::EngineContext &Context);
  void Update(Engine::EngineContext &Context, float DeltaTime);
  void Draw(Engine::EngineContext &Context);
  void Shutdown(Engine::EngineContext &Context);
}
