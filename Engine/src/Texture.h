#pragma once

namespace Engine
{
  // Opaque handle to a loaded texture (see AssetManager::LoadTexture). Game/widget code only ever
  // holds and passes this pointer around; only Engine's own .cpp files give it a real definition
  // and convert to/from the real SDL_Texture it wraps.
  struct Texture;
}
