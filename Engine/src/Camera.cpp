#include "Camera.h"

namespace Engine
{
  Camera::Camera(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t Zoom)
      : Window(Window), Renderer(Renderer), ScreenWidth(ScreenWidth), ScreenHeight(ScreenHeight), Zoom(Zoom), Position{0.0f, 0.0f} {}

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
}
