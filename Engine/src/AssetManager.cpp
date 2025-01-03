#include "AssetManager.h"

namespace Engine
{
  // Initialize the static unique pointer
  std::unique_ptr<AssetManager> AssetManager::Instance = nullptr;

  AssetManager::AssetManager()
  {
    Assets.reserve(256);
  }

  AssetManager::~AssetManager()
  {
    ClearAssets();
  }

  AssetManager &AssetManager::GetInstance()
  {
    if (!Instance)
    {
      Instance.reset(new AssetManager());
    }
    return *Instance;
  }

  void AssetManager::Initialize(SDL_Renderer *NewRenderer)
  {
    Renderer = NewRenderer;
  }

  SDL_Texture *AssetManager::LoadTexture(const std::string &FilePath)
  {
    // Check if the texture is already loaded
    auto AssetIterator = Assets.find(FilePath);
    if (AssetIterator != Assets.end())
    {
      return AssetIterator->second.get();
    }

    // Load texture using SDL_image
    SDL_Texture *LoadedTexture = IMG_LoadTexture(Renderer, FilePath.c_str());
    if (!LoadedTexture)
    {
      SDL_Log("Failed to load texture '%s': %s", FilePath.c_str(), SDL_GetError());
      return nullptr;
    }

    // Store in the cache
    Assets[FilePath] = std::unique_ptr<SDL_Texture, SDL_TextureDeleter>(LoadedTexture);
    return LoadedTexture;
  }

  void AssetManager::ClearAssets()
  {
    Assets.clear();
  }
}
