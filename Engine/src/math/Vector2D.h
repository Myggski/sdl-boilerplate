#pragma once

#include "Core.h"

namespace Engine
{
  struct ENGINE_API Vector2D
  {
  public:
    Vector2D() = default;
    Vector2D(float X, float Y) : X(X), Y(Y) {}

    Vector2D operator+(const Vector2D &Other) const
    {
      return Vector2D(X + Other.X, Y + Other.Y);
    }

    Vector2D operator-(const Vector2D &Other) const
    {
      return Vector2D(X - Other.X, Y - Other.Y);
    }

    Vector2D operator*(float Scalar) const
    {
      return Vector2D(X * Scalar, Y * Scalar);
    }

    Vector2D operator/(float Scalar) const
    {
      return Vector2D(X / Scalar, Y / Scalar);
    }

  public:
    float X = 0.0f;
    float Y = 0.0f;
  };
}