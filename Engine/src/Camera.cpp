#include "Camera.h"
#include <stdexcept>

namespace Engine
{
  std::unique_ptr<Camera> Camera::MainCameraInstance = nullptr;
  SDL_Window *Camera::Window = nullptr;
  SDL_Renderer *Camera::Renderer = nullptr;

  Camera &Camera::GetMainCamera()
  {
    // Create the singleton instance only once
    if (!MainCameraInstance)
    {
      throw std::runtime_error("Camera has not been initialized yet!");
    }

    return *MainCameraInstance;
  }

  void Camera::Update(float DeltaX, float DeltaY)
  {
    Position.x += DeltaX;
    Position.y += DeltaY;
  }

  void Camera::SetZoom(uint8_t NewZoom)
  {
    // Clamp zoom between 1x (minimum) and 12x (maximum)
    if (NewZoom < 1)
    {
      NewZoom = 1; // Prevent zooming out too much
    }
    else if (NewZoom > 12)
    {
      NewZoom = 12; // Prevent zooming in too much
    }

    Zoom = NewZoom; // Accept the zoom level if it's within the valid range
  }

  void Camera::PreRender()
  {
    // Set the clear color (black background)
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);

    // Clear the screen
    SDL_RenderClear(Renderer);

    SetZoomScale();

    // Define and set the viewport based on the camera's position
    SDL_Rect Viewport{static_cast<int>(Position.x), static_cast<int>(Position.y),
                      static_cast<int>(ScreenWidth), static_cast<int>(ScreenHeight)};
    SDL_RenderSetViewport(Renderer, &Viewport);
  }

  void Camera::PostRender()
  {
    // Present the rendered frame to the screen
    SDL_RenderPresent(Renderer);
  }

  void Camera::Reset()
  {
    ResetScale();
    SDL_RenderSetViewport(Renderer, nullptr);
  }

  void Camera::ResetScale()
  {
    SDL_RenderSetScale(Renderer, 1.0f, 1.0f);
  }

  void Camera::SetZoomScale()
  {
    // Apply the camera's transformations (scale and position)
    SDL_RenderSetScale(Renderer, Zoom, Zoom);
  }

  void Camera::Initialize(SDL_Window *SDLWindow, SDL_Renderer *SDLRenderer, uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t Zoom)
  {
    if (!MainCameraInstance)
    {
      Window = SDLWindow;     // Assign the static window
      Renderer = SDLRenderer; // Assign the static renderer
      MainCameraInstance = std::make_unique<Camera>(ScreenWidth, ScreenHeight, Zoom);
    }
  }

};