#include "BoxContainer.h"
#include <algorithm>

namespace Engine::UI
{
  namespace
  {
    // Small helpers so the layout algorithm below is written once, in terms of "main"/"cross"
    // axis, rather than duplicated for Width/X (horizontal) vs Height/Y (vertical).
    float Main(bool Horizontal, Size S) { return Horizontal ? S.Width : S.Height; }
    float Cross(bool Horizontal, Size S) { return Horizontal ? S.Height : S.Width; }
    Size MakeSize(bool Horizontal, float MainValue, float CrossValue)
    {
      return Horizontal ? Size{MainValue, CrossValue} : Size{CrossValue, MainValue};
    }
    Rect MakeRect(bool Horizontal, const Rect &Base, float MainPos, float CrossPos, float MainSize, float CrossSize)
    {
      if (Horizontal)
      {
        return Rect{Base.X + MainPos, Base.Y + CrossPos, MainSize, CrossSize};
      }
      return Rect{Base.X + CrossPos, Base.Y + MainPos, CrossSize, MainSize};
    }
  }

  BoxContainer::BoxContainer() = default;
  BoxContainer::~BoxContainer() = default;

  Widget *BoxContainer::AddSlot(std::unique_ptr<Widget> Content, SizeRule Rule, float FillWeight, Alignment CrossAlignment, float Padding)
  {
    Widget *Result = Content.get();
    Slots.push_back(BoxSlot{std::move(Content), Rule, FillWeight, Padding, CrossAlignment});
    return Result;
  }

  Size BoxContainer::Measure(Size AvailableSize)
  {
    const bool Horizontal = IsHorizontal();
    const float MainAvailable = Main(Horizontal, AvailableSize);
    const float CrossAvailable = Cross(Horizontal, AvailableSize);

    float TotalMain = 0.0f;
    float MaxCross = 0.0f;

    for (BoxSlot &Slot : Slots)
    {
      if (!Slot.Content || !Slot.Content->Visible)
      {
        continue;
      }

      Size Measured = Slot.Content->Measure(MakeSize(Horizontal, MainAvailable, CrossAvailable));
      MaxCross = std::max(MaxCross, Cross(Horizontal, Measured));

      if (Slot.Rule == SizeRule::Auto)
      {
        TotalMain += Main(Horizontal, Measured) + Slot.Padding * 2.0f;
      }
      else
      {
        TotalMain += Slot.Padding * 2.0f;
      }
    }

    return MakeSize(Horizontal, TotalMain, MaxCross);
  }

  void BoxContainer::Arrange(Rect FinalRect)
  {
    ComputedRect = FinalRect;

    const bool Horizontal = IsHorizontal();
    const float MainSize = Main(Horizontal, Size{FinalRect.Width, FinalRect.Height});
    const float CrossSize = Cross(Horizontal, Size{FinalRect.Width, FinalRect.Height});

    // First pass: work out how much space each slot actually gets along the main axis.
    std::vector<float> SlotMainSizes(Slots.size(), 0.0f);
    float SumAutoMain = 0.0f;
    float SumFillWeight = 0.0f;

    for (size_t Index = 0; Index < Slots.size(); ++Index)
    {
      BoxSlot &Slot = Slots[Index];
      if (!Slot.Content || !Slot.Content->Visible)
      {
        continue;
      }

      if (Slot.Rule == SizeRule::Auto)
      {
        Size Measured = Slot.Content->Measure(MakeSize(Horizontal, MainSize, CrossSize));
        SlotMainSizes[Index] = Main(Horizontal, Measured);
        SumAutoMain += SlotMainSizes[Index] + Slot.Padding * 2.0f;
      }
      else
      {
        SumFillWeight += Slot.FillWeight;
        SumAutoMain += Slot.Padding * 2.0f; // padding still costs space even for Fill slots
      }
    }

    float RemainingForFill = std::max(0.0f, MainSize - SumAutoMain);

    for (size_t Index = 0; Index < Slots.size(); ++Index)
    {
      BoxSlot &Slot = Slots[Index];
      if (Slot.Rule == SizeRule::Fill && SumFillWeight > 0.0f)
      {
        SlotMainSizes[Index] = RemainingForFill * (Slot.FillWeight / SumFillWeight);
      }
    }

    // Second pass: position each visible slot in order along the main axis, and place it along
    // the cross axis per its CrossAlignment.
    float Cursor = 0.0f;

    for (size_t Index = 0; Index < Slots.size(); ++Index)
    {
      BoxSlot &Slot = Slots[Index];
      if (!Slot.Content || !Slot.Content->Visible)
      {
        continue;
      }

      Cursor += Slot.Padding;

      float ThisMainSize = SlotMainSizes[Index];
      float ChildCross = CrossSize;
      float CrossPos = 0.0f;

      if (Slot.CrossAlignment != Alignment::Fill)
      {
        Size Measured = Slot.Content->Measure(MakeSize(Horizontal, ThisMainSize, CrossSize));
        ChildCross = Cross(Horizontal, Measured);

        switch (Slot.CrossAlignment)
        {
        case Alignment::Center:
          CrossPos = (CrossSize - ChildCross) * 0.5f;
          break;
        case Alignment::End:
          CrossPos = CrossSize - ChildCross;
          break;
        case Alignment::Start:
        default:
          CrossPos = 0.0f;
          break;
        }
      }

      Slot.Content->Arrange(MakeRect(Horizontal, FinalRect, Cursor, CrossPos, ThisMainSize, ChildCross));

      Cursor += ThisMainSize + Slot.Padding;
    }
  }

  void BoxContainer::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    for (BoxSlot &Slot : Slots)
    {
      if (Slot.Content && Slot.Content->Visible)
      {
        Slot.Content->Render(Renderer);
      }
    }
  }

  Widget *BoxContainer::HitTest(float X, float Y)
  {
    if (!Visible)
    {
      return nullptr;
    }

    // Children in reverse order: whatever's added last is drawn last, i.e. on top, so it should
    // get first refusal at claiming the hit.
    for (auto SlotIterator = Slots.rbegin(); SlotIterator != Slots.rend(); ++SlotIterator)
    {
      if (SlotIterator->Content)
      {
        if (Widget *Hit = SlotIterator->Content->HitTest(X, Y))
        {
          return Hit;
        }
      }
    }

    // No child claimed it; fall back to the base check (only claims if this container itself has
    // BlocksInput set, e.g. a modal backdrop).
    return Widget::HitTest(X, Y);
  }
}
