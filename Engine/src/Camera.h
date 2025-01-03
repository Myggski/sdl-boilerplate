#pragma once

#include "Core.h"
#include <SDL.h>
#include <cmath>
#include <memory>

namespace Engine
{
  class ENGINE_API Camera
  {
  public:
    Camera(uint16_t ScreenWidth = 320, uint16_t ScreenHeight = 180, uint8_t Zoom = 6.f)
        : ScreenWidth(ScreenWidth), ScreenHeight(ScreenHeight), Zoom(Zoom), Position{0.0f, 0.0f} {}
    // Deleted constructor to prevent instantiation from outside
    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;

    static void Initialize(SDL_Window *SDLWindow, SDL_Renderer *SDLRenderer, uint16_t ScreenWidth = 320, uint16_t ScreenHeight = 180, uint8_t Zoom = 6);

    // Static method to access the main camera instance
    static Camera &GetMainCamera();

    // Update the camera position based on some offset
    void Update(float DeltaX, float DeltaY);

    // Set the zoom level
    void SetZoom(uint8_t NewZoom);

    // Get the current zoom level
    float GetZoom() const { return Zoom; }

    // Get the current camera position
    SDL_FPoint GetPosition() const { return Position; }

    // Prepare the camera for rendering (clear screen and apply transformations)
    void PreRender();

    // Finalize rendering with the camera (present the frame)
    void PostRender();

    // Reset the camera transformations (optional)
    void Reset();

    void ResetScale();

    void SetZoomScale();

  private:
    // Static unique pointer to hold the singleton instance
    static std::unique_ptr<Camera> MainCameraInstance;

    static SDL_Window *Window;
    static SDL_Renderer *Renderer;
    SDL_FPoint Position{0, 0};
    uint16_t ScreenWidth = 320;
    uint16_t ScreenHeight = 180;
    uint8_t Zoom = 6;
  };
}
