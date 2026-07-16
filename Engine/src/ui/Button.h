#pragma once

#include "Core.h"
#include "Widget.h"
#include "GameEvent.h"
#include <memory>
#include <SDL_pixels.h>

struct SDL_Texture;

namespace Engine::UI
{
  // A clickable rect with an optional content widget (an Image or Text, or a Panel or a whole
  // layout, nothing stops you from nesting one in there), rather than separate
  // IconButton/TextButton/etc. widget types. BlocksInput defaults on, unlike Panel.
  class ENGINE_API Button : public Widget
  {
  public:
    Button();
    ~Button() override;

    // Takes ownership of Content; returns a non-owning pointer, same convention as
    // BoxContainer::AddSlot. Optional: a Button with no content is just a colored/textured rect.
    // A Text widget here draws as a label on top of the background; the background (color or
    // image, see SetBackgroundImage) and Content are independent, so a button can have both an
    // image background and a text label at once.
    Widget *SetContent(std::unique_ptr<Widget> Content);

    void SetColors(SDL_Color Normal, SDL_Color Hovered, SDL_Color Pressed);
    void SetDesiredSize(Size NewSize);

    // Non-owning, same convention as Image::SetTexture; comes from AssetManager::LoadTexture and
    // outlives this widget. Pass nullptr to go back to drawing SetColors' flat fill. Stretches to
    // fill the button same as Image does; darkens slightly on press for feedback (texture tinting
    // via SDL_SetTextureColorMod can only darken, not brighten, so there is no separate hover tint).
    void SetBackgroundImage(SDL_Texture *Texture);

    // Optional: crops SetBackgroundImage to a sub-rect of the texture's own pixels (e.g. one icon
    // out of a sprite sheet), same {0,0,0,0}-means-"whole texture" convention as
    // Image::SetSourceRect.
    void SetBackgroundImageSourceRect(Rect NewSourceRect);

    Size Measure(Size AvailableSize) override;
    void Arrange(Rect FinalRect) override;
    void Render(SDL_Renderer *Renderer) override;

    void OnPointerEnter() override;
    void OnPointerLeave() override;
    void OnPointerDown() override;
    void OnPointerUp(bool StillHovered) override;

    GameEvent<> &OnPressed() { return Pressed; }
    GameEvent<> &OnReleased() { return Released; }
    GameEvent<> &OnClicked() { return Clicked; }

  private:
    std::unique_ptr<Widget> Content;
    Size DesiredSize{};

    SDL_Texture *BackgroundImage = nullptr;
    Rect BackgroundImageSourceRect{};

    SDL_Color NormalColor{60, 60, 65, 255};
    SDL_Color HoveredColor{85, 85, 92, 255};
    SDL_Color PressedColor{40, 40, 45, 255};

    bool IsHovered = false;
    bool IsPressedDown = false;

    GameEvent<> Pressed;
    GameEvent<> Released;
    GameEvent<> Clicked;
  };
}
