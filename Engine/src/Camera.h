#pragma once

#include "Core.h"
#include "Texture.h"
#include "math/Rect.h"
#include <SDL3/SDL.h>
#include <cmath>

namespace Engine
{
  class ENGINE_API Camera
  {
  public:
    Camera(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth = 320, uint16_t ScreenHeight = 180, uint8_t Zoom = 6);

    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;

    // Update the camera position based on some offset
    void Update(float DeltaX, float DeltaY);

    // Draws a sub-rect of InTexture (SourceRect) to a rect on screen (DestRect), optionally
    // rotated (degrees, clockwise, about DestRect's center); game code building sprite draws (see
    // RenderSystem) should go through this instead of calling SDL_RenderTexture/Rotated with a
    // hand-built SDL_FRect itself, same reasoning as Engine::UI::Color/Engine::Rect: raw SDL
    // types/calls stay inside the engine.
    void DrawSprite(Texture *InTexture, Rect SourceRect, Rect DestRect, float RotationDegrees = 0.0f);

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
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_FPoint Position{0, 0};
    uint16_t ScreenWidth = 320;
    uint16_t ScreenHeight = 180;
    uint8_t Zoom = 6;
  };
}
