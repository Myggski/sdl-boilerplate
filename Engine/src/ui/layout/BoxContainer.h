#pragma once

#include "Core.h"
#include "../Widget.h"
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

    // Applied by the single-argument AddSlot(Content) overload below, so a container whose
    // children mostly share the same SizeRule/FillWeight/CrossAlignment/Padding (e.g. a toolbar
    // of same-sized, same-spaced buttons) only needs to say so once instead of repeating all four
    // on every single AddSlot call. Still overridable per-slot via the other AddSlot overload.
    BoxContainer *SetDefaultSizeRule(SizeRule NewRule)
    {
      DefaultRule = NewRule;
      return this;
    }
    BoxContainer *SetDefaultFillWeight(float NewWeight)
    {
      DefaultFillWeight = NewWeight;
      return this;
    }
    BoxContainer *SetDefaultCrossAlignment(Alignment NewAlignment)
    {
      DefaultCrossAlignment = NewAlignment;
      return this;
    }
    BoxContainer *SetDefaultPadding(float NewPadding)
    {
      DefaultPadding = NewPadding;
      return this;
    }

    // Takes ownership of Content; returns a non-owning pointer of the same concrete type that was
    // passed in (e.g. AddSlot(CreateWidget<Button>()) returns Button*, not Widget*), so the
    // caller can keep wiring it up (OnClicked(), etc.) without a manual cast. Template rather than
    // a plain Widget* parameter purely for that return-type inference; there is nothing generic
    // about the implementation itself. Has to live here in the header, not BoxContainer.cpp,
    // templates need to be visible wherever they're instantiated.
    //
    // Three overloads, all falling back to this container's own SetDefault* values (Auto/1.0/
    // Fill/0 unless changed) for whatever isn't given explicitly: no args beyond Content uses all
    // four defaults, Rule alone overrides just the one value most likely to differ slot-to-slot
    // (e.g. one Fill spacer in an otherwise-Auto toolbar), and the full form overrides everything.
    // The three don't have default *parameter* values layered on one signature on purpose: with
    // Rule made optional, a 2-argument call would be ambiguous between "Rule given, rest
    // defaulted" and "Rule defaulted, FillWeight given", so this needs to be genuinely separate
    // overloads instead.
    template <typename T>
    T *AddSlot(std::unique_ptr<T> Content)
    {
      return AddSlot(std::move(Content), DefaultRule, DefaultFillWeight, DefaultCrossAlignment, DefaultPadding);
    }

    template <typename T>
    T *AddSlot(std::unique_ptr<T> Content, SizeRule Rule)
    {
      return AddSlot(std::move(Content), Rule, DefaultFillWeight, DefaultCrossAlignment, DefaultPadding);
    }

    template <typename T>
    T *AddSlot(std::unique_ptr<T> Content, SizeRule Rule, float FillWeight, Alignment CrossAlignment, float Padding)
    {
      T *Result = Content.get();
      Slots.push_back(BoxSlot{std::move(Content), Rule, FillWeight, Padding, CrossAlignment});
      return Result;
    }

    // Removes and destroys every slot, for widgets that rebuild their children dynamically (e.g.
    // Dropdown repopulating its option list on SetOptions).
    BoxContainer *ClearSlots();

    Size Measure(Size AvailableSize) override;
    void Arrange(Rect FinalRect) override;
    void Render(SDL_Renderer *Renderer) override;
    Widget *HitTest(float X, float Y) override;

  protected:
    virtual bool IsHorizontal() const = 0;

  private:
    SizeRule DefaultRule = SizeRule::Auto;
    float DefaultFillWeight = 1.0f;
    Alignment DefaultCrossAlignment = Alignment::Fill;
    float DefaultPadding = 0.0f;

    std::vector<BoxSlot> Slots;
  };
}
