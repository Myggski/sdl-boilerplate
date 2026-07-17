#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"

namespace Engine::UI
{
  // Simplest renderable widget: a solid-color rect. Useful on its own (backgrounds, dividers,
  // placeholders while building out real content) and as the starting point Button/Image will
  // build on next.
  class ENGINE_API Panel : public Widget
  {
  public:
    Panel();
    explicit Panel(Color PanelColor);
    ~Panel() override;

    Panel *SetColor(Color NewColor);

    // {0,0} (the default) means "no intrinsic size, take whatever a Fill slot gives you."
    Panel *SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

  private:
    Color PanelColor = Theme::Surface;
    Size DesiredSize{};
  };
}
