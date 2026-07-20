#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"
#include "GameEvent.h"
#include <memory>
#include <string>
#include <vector>

struct TTF_Font;

namespace Engine
{
  class AssetManager;
}

namespace Engine::UI
{
  class VerticalBox;
  class Button;
  class Text;

  // A header Button showing the selected option; click it to open a list of option Buttons below
  // it, click one to select it and close.
  //
  // Only the header participates in normal layout (Measure/Arrange forward to it alone, so
  // opening never reflows surrounding widgets); the options list is positioned and
  // rendered/hit-tested separately, outside normal layout flow, since Canvas has no
  // popup/layering system yet. Limitation: the open list can still be drawn over by anything
  // rendered after this Dropdown in tree order.
  class ENGINE_API Dropdown : public Widget
  {
  public:
    Dropdown();
    ~Dropdown() override;

    Dropdown *SetFont(TTF_Font *NewFont);

    // Convenience: loads the font (and applies the color) from Theme::TextStyle in one call,
    // rather than the caller pre-loading a TTF_Font* and calling SetFont/SetTextColor separately.
    Dropdown *SetFont(Engine::AssetManager &Assets, Theme::TextStyle Style = {});

    Dropdown *SetOptions(std::vector<std::string> NewOptions);
    Dropdown *SetSelectedIndex(int NewIndex);
    int GetSelectedIndex() const { return SelectedIndex; }

    Dropdown *SetColors(Color Normal, Color Hovered, Color Pressed);

    // Background of the option in the open list matching the current selection, so it reads as
    // "this is your pick" rather than a plain duplicate of the header.
    Dropdown *SetSelectedOptionColor(Color NewColor);

    Dropdown *SetTextColor(Color NewColor);

    // Minimum width Measure() reports, not a fixed/exact size. Height always comes from the
    // header alone (see class comment).
    Dropdown *SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Arrange(Rect FinalRect) override;
    void Render(SDL_Renderer *Renderer) override;
    Widget *HitTest(float X, float Y) override;

    GameEvent<int> &OnSelectionChanged() { return SelectionChanged; }

  private:
    void RebuildOptionButtons();
    void RecolorOptionButtons();
    void SetOpen(bool NewOpen);
    void SelectOption(int Index);

    std::unique_ptr<Button> Header;
    Text *HeaderLabel = nullptr;
    std::unique_ptr<VerticalBox> OptionsBox;

    // Non-owning; OptionsBox's slots own the actual Buttons. Kept so SelectOption can recolor in
    // place instead of rebuilding, which would destroy the very Button whose OnClicked callback
    // is currently executing SelectOption (use-after-free).
    std::vector<Button *> OptionButtons;

    TTF_Font *Font = nullptr;
    std::vector<std::string> Options;
    int SelectedIndex = -1;
    bool IsOpen = false;

    Size DesiredSize{160.0f, Theme::InputHeight};
    Color NormalColor = Theme::NeutralNormal;

    // Primary* family, not Neutral*, so hover/press feedback matches SelectedOptionColor's family.
    Color HoveredColor = Theme::PrimaryHovered;
    Color PressedColor = Theme::PrimaryPressed;

    Color SelectedOptionColor = Theme::PrimaryNormal;
    Color TextColor = Theme::TextPrimary;

    GameEvent<int> SelectionChanged;
  };
}
