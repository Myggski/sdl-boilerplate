#include "Toolbar.h"
#include "AssetPaths.h"
#include "src/Log.h"
#include "src/ui/Widget.h"
#include "src/ui/Theme.h"
#include "src/ui/display/Panel.h"
#include "src/ui/display/Text.h"
#include "src/ui/layout/VerticalBox.h"
#include "src/ui/layout/HorizontalBox.h"
#include "src/ui/input/Button.h"
#include "src/ui/input/Checkbox.h"
#include "src/ui/input/Scalar.h"
#include "src/ui/input/Dropdown.h"
#include "src/ui/input/TextInput.h"

namespace Game
{
  using namespace Engine;
  using namespace Engine::UI;
  using namespace Engine::UI::Theme;

  void BuildToolbar(EngineContext &Context)
  {
    // Static: the callbacks below run long after this function returns, so Indicator must outlive it.
    static Panel *Indicator = nullptr;

    std::unique_ptr<VerticalBox> CornerBox = CreateWidget<VerticalBox>();
    std::unique_ptr<Panel> IndicatorPanel = CreateWidget<Panel>(Accent);
    IndicatorPanel->SetDesiredSize({Spacing::XLarge, Spacing::XLarge});
    Indicator = CornerBox->AddSlot(std::move(IndicatorPanel), SizeRule::Auto, 1.0f, Alignment::Start, Spacing::Medium);
    Context.UICanvas.AddRoot(std::move(CornerBox));

    std::unique_ptr<VerticalBox> RootBox = CreateWidget<VerticalBox>();
    RootBox->AddSlot(CreateWidget<Widget>(), SizeRule::Fill);

    std::unique_ptr<HorizontalBox> ToolbarBox = CreateWidget<HorizontalBox>();
    ToolbarBox->SetDefaultCrossAlignment(Alignment::Center)->SetDefaultPadding(Spacing::Small);
    HorizontalBox *Toolbar = ToolbarBox.get();

    auto AddToolbarButton = [Toolbar](Color Normal, Color Hovered, Color Pressed)
    {
      std::unique_ptr<Button> NewButton = CreateWidget<Button>();
      NewButton->SetDesiredSize({MinTouchTarget, MinTouchTarget})->SetColors(Normal, Hovered, Pressed);
      return Toolbar->AddSlot(std::move(NewButton));
    };

    auto ToggleIndicator = []()
    { Indicator->Visible = !Indicator->Visible; };

    Button *LabelButton = AddToolbarButton(DangerNormal, DangerHovered, DangerPressed);
    LabelButton->SetContent(CreateLabel(Context.Assets, "Hi", TextStyles::Header3));
    LabelButton->SetContentPadding({.Left = 12.0f, .Top = 8.0f, .Right = 12.0f, .Bottom = 8.0f});
    LabelButton->OnClicked().Add(ToggleIndicator);

    Button *ImageButton = AddToolbarButton(SuccessNormal, SuccessHovered, SuccessPressed);
    ImageButton->SetBackgroundImage(Context.Assets.LoadTexture(PlayerTexturePath));
    ImageButton->OnClicked().Add(ToggleIndicator);

    AddToolbarButton(PrimaryNormal, PrimaryHovered, PrimaryPressed)->OnClicked().Add(ToggleIndicator);

    std::unique_ptr<Checkbox> CheckboxWidget = CreateWidget<Checkbox>();
    CheckboxWidget->OnCheckedChanged().Add([](bool NewChecked)
                                           { ENGINE_LOG_INFO("Checkbox toggled: %s", NewChecked ? "true" : "false"); });
    Toolbar->AddSlot(std::move(CheckboxWidget));

    std::unique_ptr<Scalar> ScalarWidget = CreateWidget<Scalar>();
    ScalarWidget->SetFont(Context.Assets)
        ->SetRange(-10.0f, 10.0f)
        ->SetStep(1.0f)
        ->OnValueChanged()
        .Add([](float NewValue)
             { ENGINE_LOG_INFO("Scalar value: %.0f", NewValue); });
    Toolbar->AddSlot(std::move(ScalarWidget));

    std::unique_ptr<Dropdown> DropdownWidget = CreateWidget<Dropdown>();
    DropdownWidget->SetFont(Context.Assets)
        ->SetOptions({"Yellow", "Red", "Blue"})
        ->OnSelectionChanged()
        .Add([](int NewIndex)
             {
      static const Color Colors[3] = {Accent, DangerHovered, PrimaryHovered};
      Indicator->SetColor(Colors[NewIndex]);
      ENGINE_LOG_INFO("Dropdown selection: %d", NewIndex); });
    Toolbar->AddSlot(std::move(DropdownWidget));

    std::unique_ptr<TextInput> NameInput = CreateWidget<TextInput>();
    NameInput->SetFont(Context.Assets)
        ->SetPlaceholder("Enter your name")
        ->SetMaxLength(20)
        ->OnSubmitted()
        .Add([]()
             { ENGINE_LOG_INFO("Name submitted"); });
    Toolbar->AddSlot(std::move(NameInput));

    RootBox->SetDefaultCrossAlignment(Alignment::Center)->SetDefaultPadding(192.0f);
    RootBox->AddSlot(std::move(ToolbarBox));

    Context.UICanvas.AddRoot(std::move(RootBox));
  }
}
