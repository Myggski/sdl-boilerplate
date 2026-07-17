#include "Canvas.h"
#include "../InputManager.h"
#include <SDL_render.h>
#include <SDL_mouse.h>

namespace Engine::UI
{
  namespace
  {
    Rect FullScreenRectFor(SDL_Renderer *Renderer, Size &OutSize)
    {
      int ScreenWidth = 0;
      int ScreenHeight = 0;
      SDL_GetRendererOutputSize(Renderer, &ScreenWidth, &ScreenHeight);

      OutSize = Size{static_cast<float>(ScreenWidth), static_cast<float>(ScreenHeight)};
      return Rect{0.0f, 0.0f, OutSize.Width, OutSize.Height};
    }
  }

  Canvas::Canvas() = default;
  Canvas::~Canvas() = default;

  void Canvas::UpdateLayout(SDL_Renderer *Renderer)
  {
    Size FullScreen;
    Rect FullScreenRect = FullScreenRectFor(Renderer, FullScreen);

    for (std::unique_ptr<Widget> &Root : Roots)
    {
      if (!Root || !Root->Visible)
      {
        continue;
      }

      Root->Measure(FullScreen);
      Root->Arrange(FullScreenRect);
    }
  }

  void Canvas::ProcessInput(InputManager &Input)
  {
    float MouseX = static_cast<float>(Input.GetMouseX());
    float MouseY = static_cast<float>(Input.GetMouseY());
    float DeltaX = MouseX - LastMouseX;
    float DeltaY = MouseY - LastMouseY;
    LastMouseX = MouseX;
    LastMouseY = MouseY;

    // Later roots are drawn on top, so they get first refusal at the hit.
    Widget *HitWidget = nullptr;
    for (auto RootIterator = Roots.rbegin(); RootIterator != Roots.rend(); ++RootIterator)
    {
      if (*RootIterator && (*RootIterator)->Visible)
      {
        HitWidget = (*RootIterator)->HitTest(MouseX, MouseY);
        if (HitWidget)
        {
          break;
        }
      }
    }

    Input.SetPointerClaimed(HitWidget != nullptr);

    if (HitWidget != HoveredWidget)
    {
      if (HoveredWidget)
      {
        HoveredWidget->OnPointerLeave();
      }
      if (HitWidget)
      {
        HitWidget->OnPointerEnter();
      }
      HoveredWidget = HitWidget;
    }

    // Bypass the claim here: this is the code that decides what gets claimed in the first place,
    // it needs the real button state regardless.
    bool LeftJustPressed = Input.IsMouseButtonJustPressed(SDL_BUTTON_LEFT, false);
    bool LeftJustReleased = Input.IsMouseButtonReleased(SDL_BUTTON_LEFT, false);

    if (LeftJustPressed && HitWidget)
    {
      PressedWidget = HitWidget;
      PressedWidget->OnPointerDown(MouseX, MouseY);
    }
    else if (LeftJustReleased && PressedWidget)
    {
      Widget *Released = PressedWidget;
      PressedWidget = nullptr;
      Released->OnPointerUp(Released == HitWidget);
    }
    else if (PressedWidget && (DeltaX != 0.0f || DeltaY != 0.0f))
    {
      PressedWidget->OnPointerDrag(MouseX, MouseY, DeltaX, DeltaY);
    }
  }

  void Canvas::Render(SDL_Renderer *Renderer)
  {
    for (std::unique_ptr<Widget> &Root : Roots)
    {
      if (Root && Root->Visible)
      {
        Root->Render(Renderer);
      }
    }
  }
}
