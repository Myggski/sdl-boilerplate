#pragma once

#include "Core.h"
#include "Widget.h"

namespace Engine::UI::Theme
{
  // The Lospec "31" palette (https://lospec.com/palette-list/31). Referenced only through the
  // semantic roles below everywhere else in the engine, not by these names directly, so a future
  // palette swap only means editing this file.
  namespace Palette
  {
    inline constexpr Color GrayDark{0x63, 0x66, 0x63, 255};
    inline constexpr Color GrayMid{0x87, 0x85, 0x7c, 255};
    inline constexpr Color GrayLight{0xbc, 0xad, 0x9f, 255};
    inline constexpr Color Peach{0xf2, 0xb8, 0x88, 255};
    inline constexpr Color Orange{0xeb, 0x96, 0x61, 255};
    inline constexpr Color Rust{0xb5, 0x59, 0x45, 255};
    inline constexpr Color Brown{0x73, 0x4c, 0x44, 255};
    inline constexpr Color BrownDark{0x3d, 0x33, 0x33, 255};
    inline constexpr Color Mauve{0x59, 0x3e, 0x47, 255};
    inline constexpr Color MauveWarm{0x7a, 0x58, 0x59, 255};
    inline constexpr Color Tan{0xa5, 0x78, 0x55, 255};
    inline constexpr Color Gold{0xde, 0x9f, 0x47, 255};
    inline constexpr Color Yellow{0xfd, 0xd1, 0x79, 255};
    inline constexpr Color Cream{0xfe, 0xe1, 0xb8, 255};
    inline constexpr Color OliveLight{0xd4, 0xc6, 0x92, 255};
    inline constexpr Color Olive{0xa6, 0xb0, 0x4f, 255};
    inline constexpr Color GreenOlive{0x81, 0x94, 0x47, 255};
    inline constexpr Color Green{0x44, 0x70, 0x2d, 255};
    inline constexpr Color GreenDark{0x2f, 0x4d, 0x2f, 255};
    inline constexpr Color GreenGray{0x54, 0x67, 0x56, 255};
    inline constexpr Color Sage{0x89, 0xa4, 0x77, 255};
    inline constexpr Color Mint{0xa4, 0xc5, 0xaf, 255};
    inline constexpr Color MintLight{0xca, 0xe6, 0xd9, 255};
    inline constexpr Color White{0xf1, 0xf6, 0xf0, 255};
    inline constexpr Color SlateLight{0xd5, 0xd6, 0xdb, 255};
    inline constexpr Color Slate{0xbb, 0xc3, 0xd0, 255};
    inline constexpr Color BlueGray{0x96, 0xa9, 0xc1, 255};
    inline constexpr Color Blue{0x6c, 0x81, 0xa1, 255};
    inline constexpr Color BlueDark{0x40, 0x52, 0x73, 255};
    inline constexpr Color Navy{0x30, 0x38, 0x43, 255};
    inline constexpr Color NavyDark{0x14, 0x23, 0x3a, 255};
  }

  // Semantic roles, chosen by WCAG contrast ratio against TextPrimary: Normal/Pressed pairings
  // are >=4.5:1 (AA for text); Hovered can dip to ~3:1 (the looser UI-component threshold), since
  // hover is transient and mouse-only.
  inline constexpr Color Background = Palette::NavyDark;
  inline constexpr Color Surface = Palette::Navy;

  inline constexpr Color TextPrimary = Palette::White;
  inline constexpr Color TextSecondary = Palette::BlueGray;

  inline constexpr Color PrimaryNormal = Palette::BlueDark;
  inline constexpr Color PrimaryHovered = Palette::Blue;
  inline constexpr Color PrimaryPressed = Palette::Navy;

  inline constexpr Color SuccessNormal = Palette::Green;
  inline constexpr Color SuccessHovered = Palette::GreenOlive;
  inline constexpr Color SuccessPressed = Palette::GreenDark;

  inline constexpr Color DangerNormal = Palette::Brown;
  inline constexpr Color DangerHovered = Palette::Rust;
  inline constexpr Color DangerPressed = Palette::Mauve;

  inline constexpr Color NeutralNormal = Palette::GrayDark;
  inline constexpr Color NeutralHovered = Palette::GrayMid;
  inline constexpr Color NeutralPressed = Palette::BrownDark;

  // Standalone highlight/warning color, not the interactive accent - fill bars, checkmarks, and
  // selection highlights use PrimaryNormal instead (gold reads as a warning, not a state).
  inline constexpr Color Accent = Palette::Gold;

  // 8px base unit grid. Spacing/sizing discipline only, not a visual style (Material's chrome -
  // shadows, ripples, rounded corners - would clash with the pixel-art palette).
  namespace Spacing
  {
    inline constexpr float XSmall = 4.0f;
    inline constexpr float Small = 8.0f;
    inline constexpr float Medium = 16.0f;
    inline constexpr float Large = 24.0f;
    inline constexpr float XLarge = 32.0f;
    inline constexpr float XXLarge = 48.0f;
    inline constexpr float XXXLarge = 64.0f;
  }

  // Minimum interactive touch/click target size. Matches Spacing::XXLarge; kept as its own name
  // since the two mean different things even though numerically equal today.
  inline constexpr float MinTouchTarget = 48.0f;

  // Shared row height for single-line data-entry controls (Scalar, Dropdown, TextInput), so a
  // form/toolbar lining them up side by side reads as one family. Doesn't cover Checkbox (a small
  // toggle) or Button (spans icon buttons to large touch targets).
  inline constexpr float InputHeight = Spacing::XLarge;

  // Default text appearance for widgets that build a Text label on the caller's behalf, so a call
  // site only needs to say what differs from the default, e.g. {.PointSize = 20}.
  struct TextStyle
  {
    int PointSize = 16;
    Color Color = TextPrimary;
  };

  // A small type scale (Header1/2/3, Body, Caption), sizes stepping down ~1.25-1.3x per level.
  // Body matches TextStyle{}'s own default, so an unstyled CreateLabel() call is already "Body".
  // Only one font weight is loaded, so size and color are the only distinguishing factors.
  namespace TextStyles
  {
    inline constexpr TextStyle Header1{32, TextPrimary};
    inline constexpr TextStyle Header2{24, TextPrimary};
    inline constexpr TextStyle Header3{20, TextPrimary};
    inline constexpr TextStyle Body{16, TextPrimary};
    inline constexpr TextStyle Caption{12, TextSecondary};
  }
}
