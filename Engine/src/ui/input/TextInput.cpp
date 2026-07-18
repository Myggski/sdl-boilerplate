#include "TextInput.h"
#include "../SDLConversions.h"
#include "../../AssetManager.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_keycode.h>
#include <algorithm>
#include <cmath>

namespace Engine::UI
{
  namespace
  {
    constexpr float TextPadding = Theme::Spacing::Small;

    size_t CodepointCount(const std::string &Text)
    {
      size_t Count = 0;
      for (unsigned char Byte : Text)
      {
        if ((Byte & 0xC0) != 0x80)
        {
          ++Count;
        }
      }
      return Count;
    }

    // Walks forward counting codepoint starts, stopping once CodepointIndex of them have been
    // seen; clamped to Text.size() so an out-of-range index safely returns the end.
    size_t ByteOffsetForCodepoint(const std::string &Text, size_t CodepointIndex)
    {
      size_t Pos = 0;
      size_t Count = 0;
      while (Pos < Text.size() && Count < CodepointIndex)
      {
        ++Pos;
        while (Pos < Text.size() && (static_cast<unsigned char>(Text[Pos]) & 0xC0) == 0x80)
        {
          ++Pos;
        }
        ++Count;
      }
      return Pos;
    }

    float PixelOffsetForCodepoint(TTF_Font *Font, const std::string &Text, size_t CodepointIndex)
    {
      std::string Prefix = Text.substr(0, ByteOffsetForCodepoint(Text, CodepointIndex));
      int Width = 0;
      int Height = 0;
      TTF_GetStringSize(Font, Prefix.c_str(), Prefix.size(), &Width, &Height);
      return static_cast<float>(Width);
    }
  }

  TextInput::TextInput()
  {
    BlocksInput = true;
    WantsFocus = true;
  }

  TextInput::~TextInput()
  {
    if (DisplayTexture)
    {
      SDL_DestroyTexture(DisplayTexture);
    }
  }

  TextInput *TextInput::SetFont(TTF_Font *NewFont)
  {
    Font = NewFont;
    Dirty = true;
    return this;
  }

  TextInput *TextInput::SetFont(Engine::AssetManager &Assets, Theme::TextStyle Style)
  {
    SetFont(Assets.LoadDefaultFont(Style.PointSize));
    return SetTextColor(Style.Color);
  }

  TextInput *TextInput::SetText(const std::string &NewText)
  {
    Text = NewText;
    CursorPosition = CodepointCount(Text);
    SelectionAnchor = CursorPosition;
    Dirty = true;
    return this;
  }

  TextInput *TextInput::SetPlaceholder(const std::string &NewPlaceholder)
  {
    Placeholder = NewPlaceholder;
    Dirty = true;
    return this;
  }

  TextInput *TextInput::SetMaxLength(size_t NewMaxLength)
  {
    MaxLength = NewMaxLength;
    return this;
  }

  TextInput *TextInput::SetColors(Color Normal, Color Hovered, Color Focused)
  {
    NormalColor = Normal;
    HoveredColor = Hovered;
    FocusedColor = Focused;
    return this;
  }

  TextInput *TextInput::SetTextColor(Color NewColor)
  {
    TextColor = NewColor;
    Dirty = true;
    return this;
  }

  TextInput *TextInput::SetPlaceholderColor(Color NewColor)
  {
    PlaceholderColor = NewColor;
    Dirty = true;
    return this;
  }

  TextInput *TextInput::SetSelectionColor(Color NewColor)
  {
    SelectionColor = NewColor;
    return this;
  }

  TextInput *TextInput::SetFullBorderColor(Color NewColor)
  {
    FullBorderColor = NewColor;
    return this;
  }

  TextInput *TextInput::SetDesiredSize(Size NewSize)
  {
    DesiredSize = NewSize;
    return this;
  }

  Size TextInput::Measure(Size)
  {
    return DesiredSize;
  }

  void TextInput::GetSelectionRange(size_t &OutStart, size_t &OutEnd) const
  {
    OutStart = std::min(SelectionAnchor, CursorPosition);
    OutEnd = std::max(SelectionAnchor, CursorPosition);
  }

  std::string TextInput::GetSelectedText() const
  {
    size_t Start = 0;
    size_t End = 0;
    GetSelectionRange(Start, End);

    size_t StartByte = ByteOffsetForCodepoint(Text, Start);
    size_t EndByte = ByteOffsetForCodepoint(Text, End);
    return Text.substr(StartByte, EndByte - StartByte);
  }

  void TextInput::DeleteSelection()
  {
    size_t Start = 0;
    size_t End = 0;
    GetSelectionRange(Start, End);

    size_t StartByte = ByteOffsetForCodepoint(Text, Start);
    size_t EndByte = ByteOffsetForCodepoint(Text, End);
    Text.erase(StartByte, EndByte - StartByte);

    CursorPosition = Start;
    SelectionAnchor = Start;
    Dirty = true;
  }

  void TextInput::PasteFromClipboard()
  {
    if (!SDL_HasClipboardText())
    {
      return;
    }

    char *ClipboardText = SDL_GetClipboardText();
    if (ClipboardText)
    {
      OnTextInput(ClipboardText);
      SDL_free(ClipboardText);
    }
  }

  void TextInput::RebuildTextureIfNeeded(SDL_Renderer *Renderer)
  {
    if (!Dirty)
    {
      return;
    }

    if (DisplayTexture)
    {
      SDL_DestroyTexture(DisplayTexture);
      DisplayTexture = nullptr;
    }

    const std::string &Content = Text.empty() ? Placeholder : Text;
    Color EffectiveColor = Text.empty() ? PlaceholderColor : TextColor;

    if (Font && !Content.empty())
    {
      SDL_Surface *Surface = TTF_RenderText_Blended(Font, Content.c_str(), Content.size(), ToSDLColor(EffectiveColor));
      if (Surface)
      {
        DisplayTexture = SDL_CreateTextureFromSurface(Renderer, Surface);
        SDL_DestroySurface(Surface);
      }
    }

    Dirty = false;
  }

  void TextInput::Render(SDL_Renderer *Renderer)
  {
    if (!Visible)
    {
      return;
    }

    Color BackgroundColor = IsFocused ? FocusedColor : (IsHovered ? HoveredColor : NormalColor);

    SDL_FRect DestRect{
        ComputedRect.X,
        ComputedRect.Y,
        ComputedRect.Width,
        ComputedRect.Height};

    SDL_BlendMode PreviousBlendMode;
    SDL_GetRenderDrawBlendMode(Renderer, &PreviousBlendMode);
    SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(Renderer, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B, BackgroundColor.A);
    SDL_RenderFillRect(Renderer, &DestRect);

    if (MaxLength > 0 && CodepointCount(Text) >= MaxLength)
    {
      SDL_SetRenderDrawColor(Renderer, FullBorderColor.R, FullBorderColor.G, FullBorderColor.B, FullBorderColor.A);
      SDL_RenderRect(Renderer, &DestRect);
    }

    // Selection highlight/text/cursor can all measure wider than the box (a long pasted string,
    // a narrow field); clip to DestRect so they're cropped at the edges instead of drawing over
    // whatever's next to this widget. Render's own clip-rect calls stay SDL_Rect (int), unlike
    // the SDL_FRect fill/draw/texture calls above, SDL3 never moved clipping to float precision.
    SDL_Rect ClipRect{
        static_cast<int>(ComputedRect.X),
        static_cast<int>(ComputedRect.Y),
        static_cast<int>(ComputedRect.Width),
        static_cast<int>(ComputedRect.Height)};

    SDL_Rect PreviousClipRect;
    bool HadClip = SDL_RenderClipEnabled(Renderer);
    if (HadClip)
    {
      SDL_GetRenderClipRect(Renderer, &PreviousClipRect);
    }
    SDL_SetRenderClipRect(Renderer, &ClipRect);

    UpdateScrollOffset();
    float TextX = ComputedRect.X + TextPadding - ScrollOffsetPixels;

    // Drawn before the text texture, like every text editor's selection highlight sits behind
    // the glyphs, not on top of them.
    if (HasSelection() && Font)
    {
      size_t Start = 0;
      size_t End = 0;
      GetSelectionRange(Start, End);

      float StartX = TextX + PixelOffsetForCodepoint(Font, Text, Start);
      float EndX = TextX + PixelOffsetForCodepoint(Font, Text, End);

      SDL_FRect HighlightRect{
          StartX,
          ComputedRect.Y,
          EndX - StartX,
          ComputedRect.Height};

      SDL_SetRenderDrawColor(Renderer, SelectionColor.R, SelectionColor.G, SelectionColor.B, SelectionColor.A);
      SDL_RenderFillRect(Renderer, &HighlightRect);
    }

    RebuildTextureIfNeeded(Renderer);

    if (DisplayTexture)
    {
      float TextureWidth = 0.0f;
      float TextureHeight = 0.0f;
      SDL_GetTextureSize(DisplayTexture, &TextureWidth, &TextureHeight);

      SDL_FRect TextRect{
          TextX,
          ComputedRect.Y + (ComputedRect.Height - TextureHeight) * 0.5f,
          TextureWidth,
          TextureHeight};

      SDL_RenderTexture(Renderer, DisplayTexture, nullptr, &TextRect);
    }

    // Blinks at a fixed rate rather than tracking time-since-focus, so it doesn't need a new
    // per-frame Update hook, Render already runs every frame regardless. Hidden while there's an
    // active selection, same convention as every other text editor, the highlight is already the
    // position indicator at that point.
    bool BlinkPhaseOn = (SDL_GetTicks() / 500) % 2 == 0;
    if (IsFocused && !HasSelection() && Font && BlinkPhaseOn)
    {
      float CursorX = TextX + PixelOffsetForCodepoint(Font, Text, CursorPosition);
      int FontHeight = TTF_GetFontHeight(Font);

      SDL_FRect CursorRect{
          CursorX,
          ComputedRect.Y + (ComputedRect.Height - FontHeight) * 0.5f,
          2.0f,
          static_cast<float>(FontHeight)};

      SDL_SetRenderDrawColor(Renderer, TextColor.R, TextColor.G, TextColor.B, TextColor.A);
      SDL_RenderFillRect(Renderer, &CursorRect);
    }

    SDL_SetRenderClipRect(Renderer, HadClip ? &PreviousClipRect : nullptr);
    SDL_SetRenderDrawBlendMode(Renderer, PreviousBlendMode);
  }

  void TextInput::UpdateScrollOffset()
  {
    if (!Font)
    {
      ScrollOffsetPixels = 0.0f;
      return;
    }

    float ViewportWidth = ComputedRect.Width - TextPadding * 2.0f;
    if (ViewportWidth <= 0.0f)
    {
      return;
    }

    float CursorPixelX = PixelOffsetForCodepoint(Font, Text, CursorPosition);

    if (CursorPixelX - ScrollOffsetPixels > ViewportWidth)
    {
      ScrollOffsetPixels = CursorPixelX - ViewportWidth;
    }
    else if (CursorPixelX - ScrollOffsetPixels < 0.0f)
    {
      ScrollOffsetPixels = CursorPixelX;
    }
  }

  void TextInput::OnPointerEnter()
  {
    IsHovered = true;
  }

  void TextInput::OnPointerLeave()
  {
    IsHovered = false;
  }

  size_t TextInput::CodepointIndexForLocalX(float LocalX) const
  {
    if (!Font)
    {
      return CursorPosition;
    }

    // LocalX is relative to the widget's own visible edge; add back ScrollOffsetPixels to land
    // in the same text-space PixelOffsetForCodepoint measures in, same shift Render() applies to
    // TextX for drawing.
    float TextSpaceX = LocalX + ScrollOffsetPixels;

    if (TextSpaceX <= 0.0f)
    {
      return 0;
    }

    // Nearest-boundary hit test: measure every codepoint boundary's pixel offset and take
    // whichever is closest to the click. Not a performance concern at this UI's scale (a few
    // dozen characters at most).
    size_t Count = CodepointCount(Text);
    size_t BestIndex = Count;
    float BestDistance = -1.0f;

    for (size_t Index = 0; Index <= Count; ++Index)
    {
      float Width = PixelOffsetForCodepoint(Font, Text, Index);
      float Distance = std::fabs(TextSpaceX - Width);
      if (BestDistance < 0.0f || Distance < BestDistance)
      {
        BestDistance = Distance;
        BestIndex = Index;
      }
    }

    return BestIndex;
  }

  void TextInput::OnPointerDown(float X, float)
  {
    // Starts a fresh selection collapsed at the click: SelectionAnchor pins here, a following
    // OnPointerDrag only moves CursorPosition, so drag-to-select falls out for free.
    CursorPosition = CodepointIndexForLocalX(X - ComputedRect.X - TextPadding);
    SelectionAnchor = CursorPosition;
  }

  void TextInput::OnPointerDrag(float X, float, float, float)
  {
    CursorPosition = CodepointIndexForLocalX(X - ComputedRect.X - TextPadding);
  }

  void TextInput::OnFocusGained()
  {
    IsFocused = true;
  }

  void TextInput::OnFocusLost()
  {
    IsFocused = false;
  }

  void TextInput::OnTextInput(const std::string &Typed)
  {
    if (Typed.empty())
    {
      return;
    }

    if (HasSelection())
    {
      DeleteSelection();
    }

    size_t IncomingCount = CodepointCount(Typed);
    if (MaxLength > 0 && CodepointCount(Text) + IncomingCount > MaxLength)
    {
      return;
    }

    size_t ByteOffset = ByteOffsetForCodepoint(Text, CursorPosition);
    Text.insert(ByteOffset, Typed);
    CursorPosition += IncomingCount;
    SelectionAnchor = CursorPosition;

    Dirty = true;
    TextChanged.Broadcast(Text);
  }

  void TextInput::OnKeyDown(SDL_Scancode PressedKey, SDL_Keymod Modifiers)
  {
    bool ShiftHeld = (Modifiers & SDL_KMOD_SHIFT) != 0;
    bool CtrlHeld = (Modifiers & SDL_KMOD_CTRL) != 0;

    switch (PressedKey)
    {
    case SDL_SCANCODE_BACKSPACE:
      if (HasSelection())
      {
        DeleteSelection();
        TextChanged.Broadcast(Text);
      }
      else if (CursorPosition > 0)
      {
        size_t CursorByteOffset = ByteOffsetForCodepoint(Text, CursorPosition);
        size_t Start = CursorByteOffset - 1;
        while (Start > 0 && (static_cast<unsigned char>(Text[Start]) & 0xC0) == 0x80)
        {
          --Start;
        }

        Text.erase(Start, CursorByteOffset - Start);
        --CursorPosition;
        SelectionAnchor = CursorPosition;

        Dirty = true;
        TextChanged.Broadcast(Text);
      }
      break;

    case SDL_SCANCODE_DELETE:
      if (HasSelection())
      {
        DeleteSelection();
        TextChanged.Broadcast(Text);
      }
      else if (CursorPosition < CodepointCount(Text))
      {
        size_t Start = ByteOffsetForCodepoint(Text, CursorPosition);
        size_t End = Start + 1;
        while (End < Text.size() && (static_cast<unsigned char>(Text[End]) & 0xC0) == 0x80)
        {
          ++End;
        }

        Text.erase(Start, End - Start);

        Dirty = true;
        TextChanged.Broadcast(Text);
      }
      break;

    case SDL_SCANCODE_LEFT:
      if (!ShiftHeld && HasSelection())
      {
        size_t Start = 0;
        size_t End = 0;
        GetSelectionRange(Start, End);
        CursorPosition = Start;
      }
      else if (CursorPosition > 0)
      {
        --CursorPosition;
      }
      if (!ShiftHeld)
      {
        SelectionAnchor = CursorPosition;
      }
      break;

    case SDL_SCANCODE_RIGHT:
      if (!ShiftHeld && HasSelection())
      {
        size_t Start = 0;
        size_t End = 0;
        GetSelectionRange(Start, End);
        CursorPosition = End;
      }
      else if (CursorPosition < CodepointCount(Text))
      {
        ++CursorPosition;
      }
      if (!ShiftHeld)
      {
        SelectionAnchor = CursorPosition;
      }
      break;

    case SDL_SCANCODE_A:
      if (CtrlHeld)
      {
        SelectionAnchor = 0;
        CursorPosition = CodepointCount(Text);
      }
      break;

    case SDL_SCANCODE_C:
      if (CtrlHeld && HasSelection())
      {
        SDL_SetClipboardText(GetSelectedText().c_str());
      }
      break;

    case SDL_SCANCODE_X:
      if (CtrlHeld && HasSelection())
      {
        SDL_SetClipboardText(GetSelectedText().c_str());
        DeleteSelection();
        TextChanged.Broadcast(Text);
      }
      break;

    case SDL_SCANCODE_V:
      if (CtrlHeld)
      {
        PasteFromClipboard();
      }
      break;

    case SDL_SCANCODE_RETURN:
    case SDL_SCANCODE_KP_ENTER:
      Submitted.Broadcast();
      break;

    default:
      break;
    }
  }
}
