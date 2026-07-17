#include "Game.h"
#include "Engine.h"
#include <SDL_image.h>

#ifdef ENGINE_WITH_DEBUG_UI
#include "imgui.h"
#endif

// This is a minimal example game: one entity with a Transform and a Velocity, moved each tick by
// a system, drawn as a sprite. Extend it, or replace it with your own.
namespace Game
{
  Engine::Entity Player;
  SDL_Texture *PlayerTexture = nullptr;
  Engine::UI::Widget *Indicator = nullptr;

  void MovementSystem(Engine::EntityManager &World, float DeltaTime)
  {
    World.ForEach<Engine::TransformComponent, Engine::VelocityComponent>(
        [DeltaTime](Engine::Entity, Engine::TransformComponent &Transform, Engine::VelocityComponent &Velocity)
        {
          Transform.Position.X += Velocity.X * DeltaTime;
          Transform.Position.Y += Velocity.Y * DeltaTime;
        });
  }

  bool Startup(Engine::EngineContext &Context)
  {
    Context.World.RegisterSystem([&Context](float DeltaTime)
                                 { MovementSystem(Context.World, DeltaTime); });

    Player = Context.World.CreateEntity();
    Context.World.AddComponent<Engine::TransformComponent>(Player, {});
    Context.World.AddComponent<Engine::VelocityComponent>(Player, {1.f, 1.f});

    PlayerTexture = Context.Assets.LoadTexture("assets/images/bomb.png");
    if (!PlayerTexture)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image: %s\n", IMG_GetError());
      return false;
    }

    // Small indicator in the top-left; the action bar's middle button toggles its Visible below,
    // proof that hiding UI on a condition actually removes it from layout/hit-testing/rendering,
    // not just drawing.
    auto CornerBox = std::make_unique<Engine::UI::VerticalBox>();
    auto IndicatorPanel = std::make_unique<Engine::UI::Panel>(SDL_Color{230, 200, 60, 255});
    IndicatorPanel->SetDesiredSize({32.0f, 32.0f});
    Indicator = CornerBox->AddSlot(std::move(IndicatorPanel), Engine::UI::SizeRule::Auto, 1.0f, Engine::UI::Alignment::Start, 16.0f);
    Context.UICanvas.AddRoot(std::move(CornerBox));

    // Action bar: three real Buttons, bottom-center (a Fill spacer above pushes the Auto-sized
    // bar down; its own Padding doubles as the ~200px bottom gap, same placement as before).
    auto RootBox = std::make_unique<Engine::UI::VerticalBox>();
    RootBox->AddSlot(std::make_unique<Engine::UI::Widget>(), Engine::UI::SizeRule::Fill);

    auto ToolbarBox = std::make_unique<Engine::UI::HorizontalBox>();
    Engine::UI::HorizontalBox *Toolbar = ToolbarBox.get();

    auto AddToolbarButton = [Toolbar](SDL_Color Normal, SDL_Color Hovered, SDL_Color Pressed)
    {
      auto NewButton = std::make_unique<Engine::UI::Button>();
      NewButton->SetDesiredSize({48.0f, 48.0f});
      NewButton->SetColors(Normal, Hovered, Pressed);
      Engine::UI::Button *Result = NewButton.get();
      Toolbar->AddSlot(std::move(NewButton), Engine::UI::SizeRule::Auto, 1.0f, Engine::UI::Alignment::Center, 8.0f);
      return Result;
    };

    auto ToggleIndicator = []()
    { Indicator->Visible = !Indicator->Visible; };

    // First button: a Text label as Content, on top of the normal color background.
    Engine::UI::Button *LabelButton = AddToolbarButton(SDL_Color{200, 60, 60, 255}, SDL_Color{230, 90, 90, 255}, SDL_Color{150, 40, 40, 255});
    if (TTF_Font *LabelFont = Context.Assets.LoadDefaultFont(20))
    {
      auto Label = std::make_unique<Engine::UI::Text>();
      Label->SetFont(LabelFont);
      Label->SetText("Hi");
      Label->SetColor(SDL_Color{255, 255, 255, 255});
      LabelButton->SetContent(std::move(Label));
    }
    LabelButton->OnClicked().Add(ToggleIndicator);

    // Second button: bomb.png as a background image instead of a flat color (darkens on press).
    Engine::UI::Button *ImageButton = AddToolbarButton(SDL_Color{60, 200, 90, 255}, SDL_Color{90, 230, 120, 255}, SDL_Color{40, 150, 60, 255});
    ImageButton->SetBackgroundImage(PlayerTexture);
    ImageButton->OnClicked().Add(ToggleIndicator);

    AddToolbarButton(SDL_Color{60, 90, 200, 255}, SDL_Color{90, 120, 230, 255}, SDL_Color{40, 60, 150, 255})->OnClicked().Add(ToggleIndicator);

    // Fourth entry: a Checkbox, logs its toggled state so it's easy to confirm from the console.
    auto CheckboxWidget = std::make_unique<Engine::UI::Checkbox>();
    CheckboxWidget->SetDesiredSize({32.0f, 32.0f});
    Engine::UI::Checkbox *CheckboxPtr = CheckboxWidget.get();
    Toolbar->AddSlot(std::move(CheckboxWidget), Engine::UI::SizeRule::Auto, 1.0f, Engine::UI::Alignment::Center, 8.0f);
    CheckboxPtr->OnCheckedChanged().Add([](bool NewChecked)
                                        { SDL_Log("Checkbox toggled: %s", NewChecked ? "true" : "false"); });

    RootBox->AddSlot(std::move(ToolbarBox), Engine::UI::SizeRule::Auto, 1.0f, Engine::UI::Alignment::Center, 200.0f);

    Context.UICanvas.AddRoot(std::move(RootBox));

    return true;
  }

  void Update(Engine::EngineContext &Context, float DeltaTime)
  {
    // Log pointer-claim transitions so it's easy to confirm the UI only blocks input where it
    // actually is (see Engine::InputManager::IsPointerClaimed / Engine::UI::Canvas::ProcessInput).
    static bool WasClaimed = false;
    bool IsClaimed = Context.Input.IsPointerClaimed();
    if (IsClaimed != WasClaimed)
    {
      SDL_Log("Pointer claimed by UI: %s", IsClaimed ? "true" : "false");
      WasClaimed = IsClaimed;
    }
  }

  void Draw(Engine::EngineContext &Context)
  {
    if (Engine::TransformComponent *Transform = Context.World.GetComponent<Engine::TransformComponent>(Player))
    {
      SDL_Rect SourceRect{0, 0, 16, 16};
      SDL_Rect DestRect{static_cast<int>(Transform->Position.X), static_cast<int>(Transform->Position.Y), 16, 16};
      SDL_RenderCopy(Context.Renderer, PlayerTexture, &SourceRect, &DestRect);
    }

#ifdef ENGINE_WITH_DEBUG_UI
    ImGui::ShowDemoWindow();
#endif
  }

  void Shutdown(Engine::EngineContext &Context)
  {
  }
}
