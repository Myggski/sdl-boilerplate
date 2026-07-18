#include "Game.h"
#include "Engine.h"
#include "framework/CollisionSettings.h"

#ifdef ENGINE_WITH_DEBUG_UI
#include "imgui.h"
#endif

// Minimal example game: one entity with a Transform and a Velocity, moved each tick, drawn as a
// sprite, plus a demo toolbar exercising the UI widgets. Extend it, or replace it with your own.
namespace Game
{
  using namespace Engine;
  using namespace Engine::UI;
  using namespace Engine::UI::Theme;

  Entity Player;
  Entity Prop;
  Entity Wall;
  Texture *PlayerTexture = nullptr;
  Panel *Indicator = nullptr;

  bool Startup(EngineContext &Context)
  {
    // Movement/Animation/Collision are already registered by EngineContext. Publish this game's
    // own layer matrix so CollisionSystem has something to check against.
    Context.World.SetResource<CollisionMatrix>(Matrix);

    Player = Context.World.CreateEntity();
    Context.World.AddComponent<TransformComponent>(Player, {});
    Context.World.AddComponent<VelocityComponent>(Player, {1.f, 1.f});

    PlayerTexture = Context.Assets.LoadTexture("assets/images/bomb.png");
    if (!PlayerTexture)
    {
      ENGINE_LOG_ERROR("Unable to load player texture, see the load failure above");
      return false;
    }

    // bomb.png is a 64x16, 4-frame horizontal spritesheet (16x16 per frame); Player animates
    // through all 4 while MovementSystem moves it. Engine::Rect, not the bare name: Engine::UI::Rect
    // is also in scope here (see the using-directives above) and would otherwise be ambiguous.
    Context.World.AddComponent<SpriteComponent>(Player, {PlayerTexture, Engine::Rect{0.0f, 0.0f, 16.0f, 16.0f}});
    Context.World.AddComponent<AnimationComponent>(
        Player, {MakeGridFrames(Vector2D{0.0f, 0.0f}, 16.0f, 16.0f, 4u), 0.15f});
    Context.World.AddComponent<ColliderComponent>(
        Player, ColliderComponent{8.0f, Layers::Player});

    // A second, static entity (e.g. a rock/prop): a SpriteComponent alone, no VelocityComponent
    // and no AnimationComponent, so MovementSystem's and AnimationSystem's ForEach queries both
    // naturally skip it. Placed clear of the UI toolbar/corner indicator, reusing frame 0 of the
    // same texture (no separate art needed to prove static and animated coexist).
    Prop = Context.World.CreateEntity();
    Context.World.AddComponent<TransformComponent>(Prop, {{220.0f, 60.0f}, 0.0f, {1.0f, 1.0f}});
    Context.World.AddComponent<SpriteComponent>(Prop, {PlayerTexture, Engine::Rect{0.0f, 0.0f, 16.0f, 16.0f}});
    // IsStatic = false (the default): Prop never moves, but it isn't world geometry either. If
    // marked static, Wall-vs-Prop would be silently skipped as a static-static pair.
    Context.World.AddComponent<ColliderComponent>(
        Prop, ColliderComponent{Vector2D{8.0f, 8.0f}, Layers::Prop});

    // A third entity purely for collision: no sprite, just world geometry near Player's start so
    // the Circle-AABB + static-collider path is exercised quickly rather than waiting for Player
    // to drift there at its slow 1px/s velocity.
    Wall = Context.World.CreateEntity();
    Context.World.AddComponent<TransformComponent>(Wall, {{15.0f, 15.0f}, 0.0f, {1.0f, 1.0f}});
    Context.World.AddComponent<ColliderComponent>(
        Wall, ColliderComponent{Vector2D{8.0f, 8.0f}, Layers::Terrain, /*IsStatic=*/true});

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
    ImageButton->SetBackgroundImage(PlayerTexture);
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

    return true;
  }

  void Update(EngineContext &Context, float DeltaTime)
  {
    static bool WasClaimed = false;
    bool IsClaimed = Context.Input.IsPointerClaimed();
    if (IsClaimed != WasClaimed)
    {
      ENGINE_LOG_INFO("Pointer claimed by UI: %s", IsClaimed ? "true" : "false");
      WasClaimed = IsClaimed;
    }
  }

  void Draw(EngineContext &Context)
  {
    RenderSystem(Context.World, Context.MainCamera);

#ifdef ENGINE_WITH_DEBUG_UI
    ImGui::ShowDemoWindow();
#endif
  }

  void Shutdown(EngineContext &Context)
  {
  }
}
