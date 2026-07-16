#pragma once

#include "Core.h"
#include "BoxContainer.h"

namespace Engine::UI
{
  class ENGINE_API HorizontalBox : public BoxContainer
  {
  public:
    HorizontalBox();
    ~HorizontalBox() override;

  protected:
    bool IsHorizontal() const override;
  };
}
