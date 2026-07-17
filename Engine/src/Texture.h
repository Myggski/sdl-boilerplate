#pragma once

namespace Engine
{
  // Opaque handle to a loaded texture (see AssetManager::LoadTexture). Game/widget code only ever
  // holds and passes this pointer around (Button::SetBackgroundImage, Image::SetTexture,
  // Camera::DrawSprite, ...); it is deliberately never given a real definition outside Engine's
  // own .cpp files, so there is nothing to do with one except pass it along. Those .cpp files
  // convert to/from the real SDL_Texture it wraps via reinterpret_cast at the boundary, same
  // "raw SDL type stays inside the engine" pattern as Engine::UI::Color/Engine::Rect.
  struct Texture;
}
