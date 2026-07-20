#include "Camera.h"
#include <algorithm>

namespace Engine
{
  Camera::Camera(SDL_Window *Window, SDL_Renderer *Renderer, uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t DisplayScale)
      : Window(Window), Renderer(Renderer), ScreenWidth(ScreenWidth), ScreenHeight(ScreenHeight), DisplayScale(static_cast<float>(DisplayScale)), Position{0.0f, 0.0f} {}

  void Camera::Update(float DeltaX, float DeltaY)
  {
    Position.x += DeltaX;
    Position.y += DeltaY;
  }

  void Camera::CenterOn(Vector2D WorldPosition)
  {
    Position.x = WorldPosition.X;
    Position.y = WorldPosition.Y;
  }

  Vector2D Camera::ScreenToWorld(Vector2D ScreenPixelPosition) const
  {
    Vector2D Logical{ScreenPixelPosition.X / DisplayScale, ScreenPixelPosition.Y / DisplayScale};
    return Vector2D{
        (Logical.X - static_cast<float>(ScreenWidth) / 2.0f) / Zoom + Position.x,
        (Logical.Y - static_cast<float>(ScreenHeight) / 2.0f) / Zoom + Position.y};
  }

  CameraFitResult Camera::ComputeFitToTargets(const std::vector<Vector2D> &Targets, float MinZoom, float MaxZoom, float Padding) const
  {
    Vector2D Min = Targets[0];
    Vector2D Max = Targets[0];
    for (const Vector2D &Target : Targets)
    {
      Min.X = std::min(Min.X, Target.X);
      Min.Y = std::min(Min.Y, Target.Y);
      Max.X = std::max(Max.X, Target.X);
      Max.Y = std::max(Max.Y, Target.Y);
    }

    Vector2D Center{(Min.X + Max.X) / 2.0f, (Min.Y + Max.Y) / 2.0f};

    // Checked before Padding is added, so exactly-coincident targets (e.g. two players spawned at
    // the same point) get MaxZoom instead of a needless zoom-out.
    float RawSpanX = Max.X - Min.X;
    float RawSpanY = Max.Y - Min.Y;

    float ZoomX = RawSpanX > 0.0f ? static_cast<float>(ScreenWidth) / (RawSpanX + Padding * 2.0f) : MaxZoom;
    float ZoomY = RawSpanY > 0.0f ? static_cast<float>(ScreenHeight) / (RawSpanY + Padding * 2.0f) : MaxZoom;

    float FitZoom = std::clamp(std::min(ZoomX, ZoomY), MinZoom, MaxZoom);

    return CameraFitResult{Center, FitZoom};
  }

  void Camera::DrawSprite(Texture *InTexture, Rect SourceRect, Rect DestRect, float RotationDegrees)
  {
    SDL_FRect Source{
        SourceRect.X,
        SourceRect.Y,
        SourceRect.Width,
        SourceRect.Height};

    // Camera pan/zoom apply here, per-sprite, not through the render viewport/scale - those are
    // sized for the window, not the camera, and can't vary without leaving part of it uncovered.
    SDL_FRect Dest{
        (DestRect.X - Position.x) * Zoom + static_cast<float>(ScreenWidth) / 2.0f,
        (DestRect.Y - Position.y) * Zoom + static_cast<float>(ScreenHeight) / 2.0f,
        DestRect.Width * Zoom,
        DestRect.Height * Zoom};

    SDL_RenderTextureRotated(Renderer, reinterpret_cast<SDL_Texture *>(InTexture), &Source, &Dest,
                              static_cast<double>(RotationDegrees), nullptr, SDL_FLIP_NONE);
  }

  void Camera::SetZoom(float NewZoom)
  {
    Zoom = std::clamp(NewZoom, 0.05f, 10.0f);
  }

  void Camera::PreRender()
  {
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
    SDL_RenderClear(Renderer);
    SetZoomScale();

    SDL_Rect Viewport{0, 0, static_cast<int>(ScreenWidth), static_cast<int>(ScreenHeight)};
    SDL_SetRenderViewport(Renderer, &Viewport);
  }

  void Camera::PostRender()
  {
    SDL_RenderPresent(Renderer);
  }

  void Camera::Reset()
  {
    ResetScale();
    SDL_SetRenderViewport(Renderer, nullptr);
  }

  void Camera::ResetScale()
  {
    SDL_SetRenderScale(Renderer, 1.0f, 1.0f);
  }

  void Camera::SetZoomScale()
  {
    SDL_SetRenderScale(Renderer, DisplayScale, DisplayScale);
  }
}
