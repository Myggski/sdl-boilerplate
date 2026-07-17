#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"
#include "GameEvent.h"
#include <memory>
#include <string>
#include <vector>

struct _TTF_Font;
using TTF_Font = _TTF_Font;

namespace Engine
{
  class AssetManager;
}

namespace Engine::UI
{
  class VerticalBox;
  class Button;
  class Text;

  // A header Button showing the selected option; click it to open a list of option Buttons
  // below it, click one to select it and close.
  //
  // Only the header participates in normal layout: Measure/Arrange forward to it alone, so
  // Dropdown always reports the same (closed) size to whatever container it sits in, opening it
  // never reflows surrounding widgets. The options list is a separate widget Dropdown positions
  // itself, directly below the header's own arranged rect, and only renders/hit-tests it while
  // open; this deliberately steps outside normal layout flow since there is no popup/layering
  // system yet (see Canvas) for it to float on. The one real limitation from doing it this way:
  // the open list can still be drawn over by anything else that renders after this Dropdown in
  // tree order, since there is no separate topmost layer, just draw order. Rare in practice, a
  // real floating layer is future work once Canvas grows one.
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

    // Minimum width Measure() reports (the header/option Buttons still size themselves from
    // their Text labels' natural glyph size otherwise, same as Button does with Content set);
    // not a fixed/exact size. Height always comes from the header alone (see class comment).
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

    // Non-owning; OptionsBox's slots own the actual Buttons. Kept so selecting an option (see
    // SelectOption) can recolor the affected buttons in place instead of destroying and
    // recreating them, since a rebuild would destroy the very Button whose own OnClicked
    // callback is executing SelectOption, a use-after-free (the button's Clicked GameEvent is
    // mid-broadcast, iterating its own Functions list, when the button gets deleted out from
    // under it).
    std::vector<Button *> OptionButtons;

    TTF_Font *Font = nullptr;
    std::vector<std::string> Options;
    int SelectedIndex = -1;
    bool IsOpen = false;

    Size DesiredSize{160.0f, Theme::Spacing::Large};
    Color NormalColor = Theme::NeutralNormal;

    // Theme::PrimaryHovered/PrimaryPressed (blue family), not Theme::NeutralHovered/NeutralPressed
    // (a warm gray/brown that reads unrelated next to SelectedOptionColor's blue below): hovering
    // or pressing any option should feel like the same family of feedback as "this is the
    // selected one", not a visually unrelated color.
    Color HoveredColor = Theme::PrimaryHovered;
    Color PressedColor = Theme::PrimaryPressed;

    Color SelectedOptionColor = Theme::PrimaryNormal;
    Color TextColor = Theme::TextPrimary;

    GameEvent<int> SelectionChanged;
  };
}
