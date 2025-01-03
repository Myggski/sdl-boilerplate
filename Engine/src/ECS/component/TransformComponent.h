#pragma once

#include "Core.h"
#include "src/math/Vector2D.h"

namespace Engine
{
    struct ENGINE_API TransformComponent
    {
        Vector2D Position{0.0f, 0.0f};
        Vector2D Scale{1.0f, 1.0f};
        float Rotation = 0.0f;

        TransformComponent() = default;
        TransformComponent(Vector2D Position, float Rotation, Vector2D Scale)
            : Position(Position), Rotation(Rotation), Scale(Scale) {}
    };
}
