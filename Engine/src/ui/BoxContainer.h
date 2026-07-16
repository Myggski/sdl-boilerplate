#pragma once

#include "Core.h"
#include "Widget.h"
#include <memory>
#include <vector>

namespace Engine::UI
{
  // How a slot's size along the box's main axis (horizontal for HorizontalBox, vertical for
  // VerticalBox) is determined. Auto: the child's own Measure() result. Fill: whatever's left
  // over after all Auto slots are accounted for, split between Fill slots proportional to Weight.
  enum class SizeRule
  {
    Auto,
    Fill
  };

  struct BoxSlot
  {
    std::unique_ptr<Widget> Content;
    SizeRule Rule = SizeRule::Auto;
    float FillWeight = 1.0f;
    float Padding = 0.0f;
    Alignment CrossAlignment = Alignment::Fill;
  };

  // Shared implementation for HorizontalBox/VerticalBox: a single-axis flex-style layout, lays
  // slots out in order along the main axis, sized per-slot by SizeRule, and positions each along
  // the cross axis per its CrossAlignment. HorizontalBox/VerticalBox just say which axis is which.
  class ENGINE_API BoxContainer : public Widget
  {
  public:
    BoxContainer();
    ~BoxContainer() override;

    // Takes ownership of Content; returns a non-owning pointer for the caller to keep around
    // (e.g. to wire up events on it later), same convention as AssetManager::LoadTexture.
    Widget *AddSlot(std::unique_ptr<Widget> Content, SizeRule Rule = SizeRule::Auto, float FillWeight = 1.0f, Alignment CrossAlignment = Alignment::Fill, float Padding = 0.0f);

    Size Measure(Size AvailableSize) override;
    void Arrange(Rect FinalRect) override;
    void Render(SDL_Renderer *Renderer) override;
    Widget *HitTest(float X, float Y) override;

  protected:
    virtual bool IsHorizontal() const = 0;

  private:
    std::vector<BoxSlot> Slots;
  };
}
