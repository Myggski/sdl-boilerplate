#include "Widget.h"

namespace Engine::UI
{
  Widget::Widget() = default;
  Widget::~Widget() = default;

  Size Widget::Measure(Size)
  {
    return Size{};
  }

  void Widget::Arrange(Rect FinalRect)
  {
    ComputedRect = FinalRect;
  }

  void Widget::Render(SDL_Renderer *)
  {
  }

  Widget *Widget::HitTest(float X, float Y)
  {
    if (!Visible)
    {
      return nullptr;
    }

    bool Inside = X >= ComputedRect.X && X < ComputedRect.X + ComputedRect.Width &&
                  Y >= ComputedRect.Y && Y < ComputedRect.Y + ComputedRect.Height;

    return (Inside && BlocksInput) ? this : nullptr;
  }

  void Widget::OnPointerEnter() {}
  void Widget::OnPointerLeave() {}
  void Widget::OnPointerDown() {}
  void Widget::OnPointerUp(bool) {}
}
