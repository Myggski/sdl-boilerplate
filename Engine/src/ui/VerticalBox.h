#pragma once

#include "Core.h"
#include "BoxContainer.h"

namespace Engine::UI
{
  class ENGINE_API VerticalBox : public BoxContainer
  {
  public:
    VerticalBox();
    ~VerticalBox() override;

  protected:
    bool IsHorizontal() const override;
  };
}
