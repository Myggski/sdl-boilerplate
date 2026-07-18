#pragma once

#include "Core.h"
#include "../Widget.h"
#include "../Theme.h"
#include "GameEvent.h"
#include <string>

struct TTF_Font;
struct SDL_Texture;

namespace Engine
{
  class AssetManager;
}

namespace Engine::UI
{
  // A single-line text entry field: click to focus and position the cursor, type to insert at
  // the cursor, Left/Right to move it (Shift+Left/Right to extend a selection, click-drag to
  // select by mouse), Backspace/Delete to remove around it, Ctrl+A/C/X/V for select-all and
  // clipboard. Content wider than the field scrolls horizontally to keep the cursor visible,
  // rather than clipping it out of view; a thin FullBorderColor outline appears once MaxLength is
  // reached. Shows Placeholder in PlaceholderColor whenever Text is empty, swapping to Text in
  // TextColor as soon as anything is typed. Deliberately doesn't reuse the Text widget internally
  // (Text always center-aligns with no way to query its measured width, and text fields
  // conventionally left-align instead); owns its own small texture-cache render path, closely
  // mirroring Text.cpp's.
  class ENGINE_API TextInput : public Widget
  {
  public:
    TextInput();
    ~TextInput() override;

    TextInput *SetFont(TTF_Font *NewFont);

    // Convenience: loads the font (and applies TextColor) from Theme::TextStyle in one call,
    // matching Scalar/Dropdown's equivalent overload.
    TextInput *SetFont(Engine::AssetManager &Assets, Theme::TextStyle Style = {});

    TextInput *SetText(const std::string &NewText);
    const std::string &GetText() const { return Text; }

    TextInput *SetPlaceholder(const std::string &NewPlaceholder);

    // 0 (the default) means unlimited. Counts UTF-8 codepoints, not bytes, so an accented name
    // doesn't hit the limit sooner than what's visibly typed would suggest.
    TextInput *SetMaxLength(size_t NewMaxLength);

    TextInput *SetColors(Color Normal, Color Hovered, Color Focused);
    TextInput *SetTextColor(Color NewColor);
    TextInput *SetPlaceholderColor(Color NewColor);
    TextInput *SetSelectionColor(Color NewColor);
    TextInput *SetFullBorderColor(Color NewColor);
    TextInput *SetDesiredSize(Size NewSize);

    Size Measure(Size AvailableSize) override;
    void Render(SDL_Renderer *Renderer) override;

    void OnPointerEnter() override;
    void OnPointerLeave() override;
    void OnPointerDown(float X, float Y) override;
    void OnPointerDrag(float X, float Y, float DeltaX, float DeltaY) override;

    void OnFocusGained() override;
    void OnFocusLost() override;
    void OnTextInput(const std::string &Typed) override;
    void OnKeyDown(SDL_Scancode PressedKey, SDL_Keymod Modifiers) override;

    GameEvent<const std::string &> &OnTextChanged() { return TextChanged; }
    GameEvent<> &OnSubmitted() { return Submitted; }

  private:
    void RebuildTextureIfNeeded(SDL_Renderer *Renderer);

    // Adjusts ScrollOffsetPixels (if at all) so CursorPosition's pixel offset stays within the
    // visible text area, same idea as any text field: typing/moving past either edge scrolls the
    // content rather than letting the cursor run out of view. Called once per Render(), not on
    // every mutation, cheap enough at this UI's scale and keeps the scroll math in one place.
    void UpdateScrollOffset();

    // Codepoint index nearest to a click/drag at LocalX (relative to the text's own left edge,
    // i.e. already past ComputedRect.X and the padding, but not yet past ScrollOffsetPixels;
    // this accounts for that internally). Shared by OnPointerDown/OnPointerDrag.
    size_t CodepointIndexForLocalX(float LocalX) const;

    bool HasSelection() const { return SelectionAnchor != CursorPosition; }
    void GetSelectionRange(size_t &OutStart, size_t &OutEnd) const;
    std::string GetSelectedText() const;

    // Erases the selected range, collapsing CursorPosition/SelectionAnchor to where it was.
    // Marks Dirty but does not broadcast OnTextChanged; callers that don't immediately insert
    // more text (Backspace/Delete/Cut with an active selection) need to broadcast themselves.
    void DeleteSelection();

    void PasteFromClipboard();

    std::string Text;
    std::string Placeholder;
    size_t CursorPosition = 0;   // Codepoint index into Text, 0..codepoint-count inclusive.
    size_t SelectionAnchor = 0;  // Equal to CursorPosition means no selection.
    size_t MaxLength = 0;

    TTF_Font *Font = nullptr;

    Color NormalColor = Theme::NeutralNormal;

    // Theme::PrimaryHovered (blue family), not Theme::NeutralHovered (a warm gray/brown that
    // reads unrelated next to FocusedColor's blue below), same reasoning as Dropdown's
    // HoveredColor/PressedColor. FocusedColor is the darker Theme::PrimaryPressed, same darkened
    // feedback Scalar gives while being dragged, so focus reads as "actively engaged" rather than
    // just another hover state.
    Color HoveredColor = Theme::PrimaryHovered;
    Color FocusedColor = Theme::PrimaryPressed;
    Color TextColor = Theme::TextPrimary;
    Color PlaceholderColor = Theme::TextSecondary;
    Color SelectionColor = Theme::PrimaryHovered;

    // Theme::PrimaryNormal: sits between PrimaryHovered (background while hovered) and
    // PrimaryPressed (background while focused), so the outline still reads clearly against
    // either one, while staying in the same blue family as the rest of this widget's states
    // rather than the gold Theme::Accent warning color.
    Color FullBorderColor = Theme::PrimaryNormal;

    bool IsHovered = false;
    bool IsFocused = false;

    Size DesiredSize{160.0f, Theme::InputHeight};

    // How far the visible text is shifted left (pixels) so CursorPosition stays in view once
    // content is wider than the field. Recomputed each Render() by UpdateScrollOffset().
    float ScrollOffsetPixels = 0.0f;

    SDL_Texture *DisplayTexture = nullptr;
    bool Dirty = true;

    GameEvent<const std::string &> TextChanged;
    GameEvent<> Submitted;
  };
}
