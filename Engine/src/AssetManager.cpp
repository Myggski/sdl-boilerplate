#include "AssetManager.h"

namespace Engine
{
  namespace
  {
    constexpr const char *DefaultFontPath = "assets/fonts/Roboto/Roboto-Regular.ttf";
  }

  AssetManager::AssetManager(SDL_Renderer *Renderer) : Renderer(Renderer)
  {
    Assets.reserve(256);

    if (!TTF_Init())
    {
      SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
    }
    else
    {
      TTFInitialized = true;
    }
  }

  AssetManager::~AssetManager()
  {
    ClearAssets();

    if (TTFInitialized)
    {
      TTF_Quit();
    }
  }

  Texture *AssetManager::LoadTexture(const std::string &FilePath)
  {
    // Check if the texture is already loaded
    auto AssetIterator = Assets.find(FilePath);
    if (AssetIterator != Assets.end())
    {
      return reinterpret_cast<Texture *>(AssetIterator->second.get());
    }

    // Load texture using SDL_image
    SDL_Texture *LoadedTexture = IMG_LoadTexture(Renderer, FilePath.c_str());
    if (!LoadedTexture)
    {
      SDL_Log("Failed to load texture '%s': %s", FilePath.c_str(), SDL_GetError());
      return nullptr;
    }

    // SDL3 defaults new textures to linear (bilinear) filtering, SDL2 defaulted to nearest.
    // Camera zooms sprites up several times over for this engine's pixel-art look, so linear
    // filtering blurs them, nearest keeps the crisp blocky-pixel edges the zoom is meant to show.
    SDL_SetTextureScaleMode(LoadedTexture, SDL_SCALEMODE_NEAREST);

    // Store in the cache
    Assets[FilePath] = std::unique_ptr<SDL_Texture, SDL_TextureDeleter>(LoadedTexture);
    return reinterpret_cast<Texture *>(LoadedTexture);
  }

  TTF_Font *AssetManager::LoadFont(const std::string &FilePath, int PointSize)
  {
    std::string Key = FilePath + "@" + std::to_string(PointSize);

    auto FontIterator = Fonts.find(Key);
    if (FontIterator != Fonts.end())
    {
      return FontIterator->second.get();
    }

    TTF_Font *LoadedFont = TTF_OpenFont(FilePath.c_str(), PointSize);
    if (!LoadedFont)
    {
      SDL_Log("Failed to load font '%s' at size %d: %s", FilePath.c_str(), PointSize, SDL_GetError());
      return nullptr;
    }

    Fonts[Key] = std::unique_ptr<TTF_Font, TTF_FontDeleter>(LoadedFont);
    return LoadedFont;
  }

  TTF_Font *AssetManager::LoadDefaultFont(int PointSize)
  {
    return LoadFont(DefaultFontPath, PointSize);
  }

  void AssetManager::ClearAssets()
  {
    Assets.clear();
    Fonts.clear();
  }
}
