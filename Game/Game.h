#pragma once

namespace Engine
{
  struct EngineContext;
}

// PreUpdate: once per real frame, before the fixed-step loop - read discrete input here (Update
// can run 0+ times per frame, so a one-frame edge could be missed there). Update: fixed timestep,
// simulation logic. PostUpdate: once per real frame, after the fixed-step loop - per-frame state
// that depends on this frame's simulation results (camera-follow). Draw: rendering only.
namespace Game
{
  bool Startup(Engine::EngineContext &Context);
  void PreUpdate(Engine::EngineContext &Context);
  void Update(Engine::EngineContext &Context, float DeltaTime);
  void PostUpdate(Engine::EngineContext &Context, float DeltaTime);
  void Draw(Engine::EngineContext &Context);
  void Shutdown(Engine::EngineContext &Context);
}
