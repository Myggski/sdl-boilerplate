#pragma once

#include "Core.h"
#include "Texture.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace Engine
{
  class ENGINE_API AssetManager
  {
  public:
    explicit AssetManager(SDL_Renderer *Renderer);
    ~AssetManager();

    Texture *LoadTexture(const std::string &FilePath);

    // Cached by FilePath + PointSize, since a TTF_Font is rasterized at a fixed size; asking for
    // the same file at a different size loads/keeps a second TTF_Font.
    TTF_Font *LoadFont(const std::string &FilePath, int PointSize);

    // Engine's bundled default UI font (Roboto), for widgets that just need "a font" without the
    // caller needing to know exactly where it lives on disk. Copied next to the built executable
    // by Engine/CMakeLists.txt, same as Game's own assets/.
    TTF_Font *LoadDefaultFont(int PointSize);

    void ClearAssets();

    // Delete copy semantics
    AssetManager(const AssetManager &) = delete;
    AssetManager &operator=(const AssetManager &) = delete;

  private:
    struct SDL_TextureDeleter
    {
      void operator()(SDL_Texture *Texture) const
      {
        SDL_DestroyTexture(Texture);
      }
    };

    struct TTF_FontDeleter
    {
      void operator()(TTF_Font *Font) const
      {
        TTF_CloseFont(Font);
      }
    };

    SDL_Renderer *Renderer = nullptr;
    bool TTFInitialized = false;

    using TexturePtr = std::unique_ptr<SDL_Texture, SDL_TextureDeleter>;
    std::unordered_map<std::string, TexturePtr> Assets;

    using FontPtr = std::unique_ptr<TTF_Font, TTF_FontDeleter>;
    std::unordered_map<std::string, FontPtr> Fonts;
  };
}
