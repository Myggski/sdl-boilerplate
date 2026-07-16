#include "VerticalBox.h"

namespace Engine::UI
{
  VerticalBox::VerticalBox() = default;
  VerticalBox::~VerticalBox() = default;

  bool VerticalBox::IsHorizontal() const
  {
    return false;
  }
}
