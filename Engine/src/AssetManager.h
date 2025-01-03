#pragma once

#include "Core.h"
#include <SDL.h>
#include <SDL_image.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace Engine
{
  class ENGINE_API AssetManager
  {
  public:
    ~AssetManager();

    static AssetManager &GetInstance();

    void Initialize(SDL_Renderer *NewRenderer);
    SDL_Texture *LoadTexture(const std::string &FilePath);
    void ClearAssets();

    // Delete copy semantics
    AssetManager(const AssetManager &) = delete;
    AssetManager &operator=(const AssetManager &) = delete;

  private:
    AssetManager(); // Private to enforce singleton

    struct SDL_TextureDeleter
    {
      void operator()(SDL_Texture *Texture) const
      {
        SDL_DestroyTexture(Texture);
      }
    };

    static std::unique_ptr<AssetManager> Instance;

    SDL_Renderer *Renderer = nullptr;

    using TexturePtr = std::unique_ptr<SDL_Texture, SDL_TextureDeleter>;
    std::unordered_map<std::string, TexturePtr> Assets;
  };
}
