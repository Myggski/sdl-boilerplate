#include "VelocityComponent.h"

namespace Engine
{
  VelocityComponent::VelocityComponent() = default;
  VelocityComponent::VelocityComponent(float X, float Y) : X(X), Y(Y) {}
}