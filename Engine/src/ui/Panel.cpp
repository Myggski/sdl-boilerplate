#include "Panel.h"
#include <SDL_render.h>

namespace Engine::UI
{
  Panel::Panel() = default;
  Panel::Panel(SDL_Color Color) : Color(Color) {}
  Panel::~Panel() = default;

  void Panel::SetColor(SDL_Color NewColor)
  {
    Color = NewColor;
  }

  void Panel::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
  }

  Size Panel::Measure(Size)
  {
    return DesiredSize;
  }

  void Panel::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    SDL_Rect DestRect{
        static_cast<int>(ComputedRect.X),
        static_cast<int>(ComputedRect.Y),
        static_cast<int>(ComputedRect.Width),
        static_cast<int>(ComputedRect.Height)};

    // SDL_RenderFillRect ignores alpha unless blending is explicitly enabled; save/restore the
    // renderer's blend mode so this doesn't leak into whatever renders next this frame.
    SDL_BlendMode PreviousBlendMode;
    SDL_GetRenderDrawBlendMode(Renderer, &PreviousBlendMode);
    SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(Renderer, Color.r, Color.g, Color.b, Color.a);
    SDL_RenderFillRect(Renderer, &DestRect);

    SDL_SetRenderDrawBlendMode(Renderer, PreviousBlendMode);
  }
}
