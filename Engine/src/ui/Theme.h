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

  // Semantic roles, chosen by WCAG contrast ratio (relative luminance, not eyeballed) against
  // TextPrimary: every Normal and Pressed pairing is >=4.5:1 (WCAG AA for text). Hovered dips as
  // low as ~3.1:1 on a couple of these, a deliberate allowance rather than an oversight, hover is
  // mouse-only (no keyboard/focus equivalent) and transient, so WCAG's strict text-contrast bar
  // is conventionally relaxed there as long as it still clears the 3:1 UI-component threshold.
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

  // Fill bars, checkmarks, selection highlights: anything that's a small accent rather than a
  // whole button's background.
  inline constexpr Color Accent = Palette::Gold;

  // 8px base unit grid (the one piece of Material Design's system this engine follows; its
  // specific visual chrome, drop shadows, ripples, rounded corners, would clash with a Lospec
  // pixel-art palette, so this is spacing/sizing discipline only, not a visual style). Consistent
  // paddings/gaps across widgets should pull from this rather than picking arbitrary numbers.
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

  // Minimum interactive touch/click target size: small hit areas are hard to click precisely,
  // especially for anyone with reduced fine motor control. Matches Spacing::XXLarge; kept as its
  // own name since the two mean different things even though they're numerically the same today.
  inline constexpr float MinTouchTarget = 48.0f;

  // Default text appearance for widgets that build a Text label on the caller's behalf (see
  // Engine::UI::CreateLabel, Scalar::SetFont(AssetManager&, ...), Dropdown's equivalent), so call
  // sites only need to say what actually differs from the theme's defaults, e.g.
  // CreateLabel(Assets, "Hi", {.PointSize = 20}), not repeat every field every time. A struct
  // rather than a growing parameter list specifically so adding a new field later (e.g. a
  // FontPath override) doesn't mean touching every function that takes one of these.
  struct TextStyle
  {
    int PointSize = 16;
    Color Color = TextPrimary;
  };

  // A small type scale (Header1/2/3, Body, Caption), the same idea as HTML's h1-h6+p or Material's
  // Headline/Title/Body/Label tiers, just fewer tiers, matching this engine's "start small" scope.
  // Sizes step down by roughly 1.25x-1.3x per level (a common type-scale ratio), rounded to values
  // that read cleanly rather than hitting an exact ratio. Body is the same 16pt/TextPrimary as
  // TextStyle{}'s own default, so existing CreateLabel(Assets, text) calls with no style argument
  // are already "Body" text, not a separate, different default. Caption uses TextSecondary (the
  // same de-emphasized color WPF/Material captions conventionally use), the other four use
  // TextPrimary; there is no bold/weight variant yet, only one weight of the default font is
  // loaded, so size and color are the only two things distinguishing a level for now.
  namespace TextStyles
  {
    inline constexpr TextStyle Header1{32, TextPrimary};
    inline constexpr TextStyle Header2{24, TextPrimary};
    inline constexpr TextStyle Header3{20, TextPrimary};
    inline constexpr TextStyle Body{16, TextPrimary};
    inline constexpr TextStyle Caption{12, TextSecondary};
  }
}
