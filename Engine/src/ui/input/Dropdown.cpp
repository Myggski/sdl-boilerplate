#include "Dropdown.h"
#include "Button.h"
#include "../display/Text.h"
#include "../layout/VerticalBox.h"
#include "../../AssetManager.h"
#include <algorithm>

namespace Engine::UI
{
  Dropdown::Dropdown()
  {
    Header = std::make_unique<Button>();
    Header->SetColors(NormalColor, HoveredColor, PressedColor);
    std::unique_ptr<Text> HeaderLabelWidget = std::make_unique<Text>();
    HeaderLabelWidget->SetColor(TextColor);
    HeaderLabelWidget->SetText("(no selection)");
    HeaderLabel = Header->SetContent(std::move(HeaderLabelWidget));
    Header->OnClicked().Add([this]()
                            { SetOpen(!IsOpen); });

    OptionsBox = std::make_unique<VerticalBox>();
  }

  Dropdown::~Dropdown() = default;

  Dropdown *Dropdown::SetFont(TTF_Font *NewFont)
  {
    Font = NewFont;
    HeaderLabel->SetFont(Font);
    RebuildOptionButtons();
    return this;
  }

  Dropdown *Dropdown::SetFont(Engine::AssetManager &Assets, Theme::TextStyle Style)
  {
    SetFont(Assets.LoadDefaultFont(Style.PointSize));
    return SetTextColor(Style.Color);
  }

  Dropdown *Dropdown::SetOptions(std::vector<std::string> NewOptions)
  {
    Options = std::move(NewOptions);

    // No valid selection survives the new list: default to the first option rather than leaving
    // the header on "(no selection)", same as most dropdowns default to showing something. A
    // valid existing selection (SetSelectedIndex called before SetOptions, unusual but possible)
    // is left alone.
    if (SelectedIndex < 0 || SelectedIndex >= static_cast<int>(Options.size()))
    {
      SelectedIndex = Options.empty() ? -1 : 0;
    }

    if (SelectedIndex >= 0)
    {
      HeaderLabel->SetText(Options[SelectedIndex]);
    }

    RebuildOptionButtons();
    return this;
  }

  Dropdown *Dropdown::SetSelectedIndex(int NewIndex)
  {
    if (NewIndex < 0 || NewIndex >= static_cast<int>(Options.size()))
    {
      return this;
    }

    SelectedIndex = NewIndex;
    HeaderLabel->SetText(Options[SelectedIndex]);
    RecolorOptionButtons();
    return this;
  }

  Dropdown *Dropdown::SetColors(Color Normal, Color Hovered, Color Pressed)
  {
    NormalColor = Normal;
    HoveredColor = Hovered;
    PressedColor = Pressed;
    Header->SetColors(Normal, Hovered, Pressed);
    RecolorOptionButtons();
    return this;
  }

  Dropdown *Dropdown::SetSelectedOptionColor(Color NewColor)
  {
    SelectedOptionColor = NewColor;
    RecolorOptionButtons();
    return this;
  }

  Dropdown *Dropdown::SetTextColor(Color NewColor)
  {
    TextColor = NewColor;
    HeaderLabel->SetColor(TextColor);
    RebuildOptionButtons();
    return this;
  }

  Dropdown *Dropdown::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
    return this;
  }

  void Dropdown::SetOpen(bool NewOpen)
  {
    IsOpen = NewOpen;
  }

  void Dropdown::SelectOption(int Index)
  {
    SetSelectedIndex(Index);
    SetOpen(false);
    SelectionChanged.Broadcast(Index);
  }

  void Dropdown::RebuildOptionButtons()
  {
    OptionsBox->ClearSlots();
    OptionButtons.clear();

    for (int Index = 0; Index < static_cast<int>(Options.size()); ++Index)
    {
      std::unique_ptr<Button> OptionButton = std::make_unique<Button>();
      bool IsSelected = Index == SelectedIndex;
      OptionButton->SetColors(IsSelected ? SelectedOptionColor : NormalColor, HoveredColor, PressedColor);

      std::unique_ptr<Text> OptionLabel = std::make_unique<Text>();
      OptionLabel->SetFont(Font);
      OptionLabel->SetText(Options[Index]);
      OptionLabel->SetColor(TextColor);
      OptionButton->SetContent(std::move(OptionLabel));

      int CapturedIndex = Index;
      OptionButton->OnClicked().Add([this, CapturedIndex]()
                                    { SelectOption(CapturedIndex); });

      OptionButtons.push_back(OptionsBox->AddSlot(std::move(OptionButton)));
    }
  }

  void Dropdown::RecolorOptionButtons()
  {
    for (int Index = 0; Index < static_cast<int>(OptionButtons.size()); ++Index)
    {
      bool IsSelected = Index == SelectedIndex;
      OptionButtons[Index]->SetColors(IsSelected ? SelectedOptionColor : NormalColor, HoveredColor, PressedColor);
    }
  }

  Size Dropdown::Measure(Size AvailableSize)
  {
    Size Measured = Header->Measure(AvailableSize);
    return Size{std::max(DesiredSize.Width, Measured.Width), Measured.Height};
  }

  void Dropdown::Arrange(Rect FinalRect)
  {
    ComputedRect = FinalRect;
    Header->Arrange(FinalRect);

    Size OptionsSize = OptionsBox->Measure(Size{FinalRect.Width, 0.0f});
    Rect OptionsRect{FinalRect.X, FinalRect.Y + FinalRect.Height, FinalRect.Width, OptionsSize.Height};
    OptionsBox->Arrange(OptionsRect);
  }

  void Dropdown::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    Header->Render(Renderer);

    if (IsOpen)
    {
      OptionsBox->Render(Renderer);
    }
  }

  Widget *Dropdown::HitTest(float X, float Y)
  {
    if (!Visible)
    {
      return nullptr;
    }

    if (IsOpen)
    {
      if (Widget *Hit = OptionsBox->HitTest(X, Y))
      {
        return Hit;
      }
    }

    return Header->HitTest(X, Y);
  }
}
