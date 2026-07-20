#pragma once

#include "Core.h"
#include "Texture.h"
#include "math/Rect.h"
#include "math/Vector2D.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <vector>

namespace Engine
{
  struct CameraFitResult
  {
    Vector2D Center;
    float Zoom;
  };

  class ENGINE_API Camera
  {
  public:
    // DisplayScale is the fixed real-pixels-per-logical-unit ratio; it must never change after
    // construction, or PreRender's fixed-size viewport stops covering the whole window.
    Camera(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth = 320, uint16_t ScreenHeight = 180, uint8_t DisplayScale = 6);

    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;

    void Update(float DeltaX, float DeltaY);

    // Points the camera at WorldPosition immediately, no smoothing - that's a follow-policy
    // concern for game code, not this class.
    void CenterOn(Vector2D WorldPosition);

    // Converts a raw screen pixel position into the world position under it.
    Vector2D ScreenToWorld(Vector2D ScreenPixelPosition) const;

    // Computes the center/zoom that fits every point in Targets on screen at once (their tightest
    // bounding box plus Padding), clamped to [MinZoom, MaxZoom]. Pure computation, doesn't move
    // the camera. Targets must be non-empty.
    CameraFitResult ComputeFitToTargets(const std::vector<Vector2D> &Targets, float MinZoom, float MaxZoom, float Padding = 0.0f) const;

    // Draws a sub-rect of InTexture to a screen rect, optionally rotated about its center.
    void DrawSprite(Texture *InTexture, Rect SourceRect, Rect DestRect, float RotationDegrees = 0.0f);

    // 1.0 shows exactly ScreenWidth x ScreenHeight world units; 2.0 is zoomed in twice as close.
    void SetZoom(float NewZoom);

    float GetZoom() const { return Zoom; }
    SDL_FPoint GetPosition() const { return Position; }

    void PreRender();
    void PostRender();
    void Reset();
    void ResetScale();
    void SetZoomScale();

  private:
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_FPoint Position{0, 0};
    uint16_t ScreenWidth = 320;
    uint16_t ScreenHeight = 180;
    float DisplayScale = 6.0f;
    float Zoom = 1.0f;
  };
}
