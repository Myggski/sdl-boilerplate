#pragma once

#include "Core.h"
#include "Widget.h"
#include <SDL_pixels.h>

namespace Engine::UI
{
  // Simplest renderable widget: a solid-color rect. Useful on its own (backgrounds, dividers,
  // placeholders while building out real content) and as the starting point Button/Image will
  // build on next.
  class ENGINE_API Panel : public Widget
  {
  public:
    Panel();
    explicit Panel(SDL_Color Color);
    ~Panel() override;

    void SetColor(SDL_Color NewColor);

    // {0,0} (the default) means "no intrinsic size, take whatever a Fill slot gives you."
    void SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

  private:
    SDL_Color Color{255, 255, 255, 255};
    Size DesiredSize{};
  };
}
