#include "Game.h"
#include "Engine.h"
#include "framework/CollisionSettings.h"
#include "framework/PathfindingSettings.h"
#include "framework/AssetPaths.h"
#include "framework/CameraFollow.h"
#include "framework/NetworkPlayers.h"
#include "framework/NetworkRPCs.h"
#include "framework/Toolbar.h"
#include <deque>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <windows.h>
#include <shellapi.h> // CommandLineToArgvW, see the --host/--connect parsing in Startup

#ifdef ENGINE_WITH_DEBUG_UI
#include "imgui.h"
#endif

namespace Game
{
  using namespace Engine;

  // Click detection happens in PreUpdate (real-frame cadence); Update (fixed-step, can run zero
  // times per frame) drains this queue instead of reading input directly.
  struct MoveCommand
  {
    Vector2D TargetWorldPos;
  };
  std::deque<MoveCommand> PendingMoveCommands;

  // Host only: catches a new connection up on every existing networked entity, then tells
  // everyone else about the newcomer.
  void HandleNewConnection(EngineContext &Context, uint32_t ConnectionId)
  {
    uint32_t NewId = AllocateNetworkId();
    std::optional<Entity> NewPlayer = SpawnPrefab(Context, PlayerPrefabId, NewId, ConnectionId);
    if (!NewPlayer)
    {
      return; // "Player" prefab unregistered - a linking mistake, not a runtime case
    }
    ClientAssignLocalNetworkId(Context, ConnectionId, NewId);

    // Includes the newcomer's own entity too - that's how it learns to spawn itself locally.
    Context.World.ForEach<NetworkIdComponent, PrefabComponent>(
        [&](Entity ExistingEntity, NetworkIdComponent &NetId, PrefabComponent &Prefab)
        {
          ClientSpawnNetworkEntity(Context, ConnectionId, NetId.NetworkId, Prefab.PrefabId,
                                    NetId.OwnerConnectionId, BuildEntitySnapshot(Context.World, ExistingEntity));

          PathFollowComponent *Follow = Context.World.GetComponent<PathFollowComponent>(ExistingEntity);
          if (Follow && !Follow->HasArrived && Follow->CurrentIndex < Follow->Waypoints.size())
          {
            std::vector<Vector2D> RemainingWaypoints(Follow->Waypoints.begin() + Follow->CurrentIndex, Follow->Waypoints.end());
            ClientSetPath(Context, ConnectionId, NetId.NetworkId, RemainingWaypoints);
          }
        });

    Engine::RPC::CallAllClientsExcept(Context, ConnectionId, ClientSpawnNetworkEntity, NewId,
                                       PlayerPrefabId, ConnectionId, BuildEntitySnapshot(Context.World, *NewPlayer));
  }

  // NetworkId is read before DestroyEntity, which invalidates the component it lives on.
  void HandleClosedConnection(EngineContext &Context, uint32_t ConnectionId)
  {
    std::optional<Entity> PlayerEntity = FindEntityByOwner(Context.World, ConnectionId);
    if (!PlayerEntity)
    {
      return;
    }
    NetworkIdComponent *NetId = Context.World.GetComponent<NetworkIdComponent>(*PlayerEntity);
    uint32_t DespawnedNetworkId = NetId->NetworkId;
    Context.World.DestroyEntity(*PlayerEntity);

    MulticastDespawnEntity(Context, DespawnedNetworkId);
  }

  bool Startup(EngineContext &Context)
  {
    // Temporary dev-testing flag parsing, not real UI.
    int ArgCount = 0;
    LPWSTR *Args = CommandLineToArgvW(GetCommandLineW(), &ArgCount);
    bool ConnectFailed = false;
    if (Args)
    {
      for (int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
      {
        if (wcscmp(Args[ArgIndex], L"--host") == 0 && ArgIndex + 1 < ArgCount)
        {
          uint16_t Port = static_cast<uint16_t>(_wtoi(Args[ArgIndex + 1]));
          Context.Network.StartHost(Port);
          break;
        }
        else if (wcscmp(Args[ArgIndex], L"--connect") == 0 && ArgIndex + 2 < ArgCount)
        {
          char AddressUtf8[64] = {};
          WideCharToMultiByte(CP_UTF8, 0, Args[ArgIndex + 1], -1, AddressUtf8, sizeof(AddressUtf8), nullptr, nullptr);
          uint16_t Port = static_cast<uint16_t>(_wtoi(Args[ArgIndex + 2]));
          ConnectFailed = Context.Network.Connect(AddressUtf8, Port) == 0;
          break;
        }
      }
      LocalFree(Args);
    }

    if (ConnectFailed)
    {
      ENGINE_LOG_ERROR("--connect failed (see NetworkManager's own error above) - aborting rather than silently falling back to offline play");
      return false;
    }

    if (Context.Network.IsHost())
    {
      Context.Network.OnClientConnected().Add([&Context](uint32_t ConnectionId)
                                              { HandleNewConnection(Context, ConnectionId); });
      Context.Network.OnClientDisconnected().Add([&Context](uint32_t ConnectionId)
                                                 { HandleClosedConnection(Context, ConnectionId); });
    }

    Context.World.SetResource<LocalPlayerState>({});

    // Movement/Animation/Collision are already registered by EngineContext. Publish this game's
    // own layer matrix so CollisionSystem has something to check against.
    Context.World.SetResource<CollisionMatrix>(Matrix);

    // Fails fast here rather than wherever a sprite first needs it - LoadTexture caches by path,
    // so this isn't a separate load from every other call site below, just an early check.
    if (!Context.Assets.LoadTexture(PlayerTexturePath))
    {
      ENGINE_LOG_ERROR("Unable to load player texture, see the load failure above");
      return false;
    }

    // Offline or hosting spawns immediately; a connecting client waits for the host to assign it
    // an id instead. Goes through SpawnPrefab, not SpawnPlayerEntity directly, so PrefabComponent
    // gets attached - otherwise the host's own player would never appear for later joiners.
    if (Context.Network.GetHostConnectionId() == 0)
    {
      uint32_t NewId = AllocateNetworkId();
      Context.World.GetResource<LocalPlayerState>()->NetworkId = NewId;
      if (!SpawnPrefab(Context, PlayerPrefabId, NewId, 0))
      {
        ENGINE_LOG_ERROR("Unable to spawn local player - \"Player\" prefab not registered");
        return false;
      }
    }

    // A second, static entity (e.g. a rock/prop): a SpriteComponent alone, no VelocityComponent
    // and no AnimationComponent, so MovementSystem's and AnimationSystem's ForEach queries both
    // naturally skip it. Placed clear of the UI toolbar/corner indicator, reusing frame 0 of the
    // same texture (no separate art needed to prove static and animated coexist).
    Entity Prop = Context.World.CreateEntity();
    Context.World.AddComponent<TransformComponent>(Prop, {{220.0f, 60.0f}, 0.0f, {1.0f, 1.0f}});
    Context.World.AddComponent<SpriteComponent>(Prop, {Context.Assets.LoadTexture(PlayerTexturePath), Engine::Rect{0.0f, 0.0f, 16.0f, 16.0f}});
    // IsStatic = false (the default): Prop never moves, but it isn't world geometry either. If
    // marked static, a WallSegment-vs-Prop pair (below) would be silently skipped as static-static.
    Context.World.AddComponent<ColliderComponent>(
        Prop, ColliderComponent{Vector2D{8.0f, 8.0f}, Layers::Prop});

    // A DeployableWall-layer obstacle: dynamic (IsStatic = false), but still blocks pathing per
    // PathfindingSettings.h's BlockingLayers mask, proving that mask isn't just IsStatic in
    // disguise.
    Entity DeployableWall = Context.World.CreateEntity();
    Context.World.AddComponent<TransformComponent>(DeployableWall, {{60.0f, 40.0f}, 0.0f, {1.0f, 1.0f}});
    Context.World.AddComponent<ColliderComponent>(
        DeployableWall, ColliderComponent{Vector2D{8.0f, 8.0f}, Layers::DeployableWall});

    // A chokepoint wall spanning almost the full height of the grid, one cell-sized gap left
    // open, so every path (A* or flow field) is forced to actually detour through that single
    // gap rather than walking a straight line to its goal. CellSize-aligned segment centers
    // (8, 24, 40, ... i.e. CellIndex*16 + 8) so the wall rasterizes onto whole cells cleanly.
    constexpr float WallX = 136.0f; // Cell column 8.
    constexpr int32_t GapCellY = 5; // Skip this row, world y = 5*16 + 8 = 88.
    for (int32_t CellY = 0; CellY < 12; ++CellY)
    {
      if (CellY == GapCellY)
      {
        continue;
      }

      Entity WallSegment = Context.World.CreateEntity();
      Context.World.AddComponent<TransformComponent>(
          WallSegment, {{WallX, static_cast<float>(CellY) * 16.0f + 8.0f}, 0.0f, {1.0f, 1.0f}});
      Context.World.AddComponent<ColliderComponent>(
          WallSegment, ColliderComponent{Vector2D{8.0f, 8.0f}, Layers::Terrain, /*IsStatic=*/true});
    }

    // Pathfinding: covers the same rough area the virtual 320x180 camera resolution shows,
    // CellSize 16 matches the existing colliders' own size (Player's radius, WallSegment/Prop's
    // half-extents), so obstacles rasterize cleanly onto whole cells.
    Context.World.SetResource<PathBlockingLayers>(BlockingLayers);
    RebuildNavGrid(Context.World, Vector2D{0.0f, 0.0f}, /*WidthCells*/ 20, /*HeightCells*/ 12, /*CellSize*/ 16.0f);

    // Flow field demo: a stand-in "base" a few enemies converge on, all sharing GoalId 0 (the
    // default FlowFieldFollowComponent::GoalId, so no explicit id plumbing needed for a single
    // goal). Enemies reuse PlayerTexturePath's first frame, same as Prop, no separate art needed.
    Vector2D BasePosition{280.0f, 150.0f};
    Entity Base = Context.World.CreateEntity();
    Context.World.AddComponent<TransformComponent>(Base, {BasePosition, 0.0f, {1.0f, 1.0f}});

    NavGrid *Grid = Context.World.GetResource<NavGrid>();
    Context.World.SetResource<FlowFieldSet>({});
    Context.World.GetResource<FlowFieldSet>()->Fields[0] = BuildFlowField(*Grid, BasePosition);

    for (Vector2D SpawnPos : {Vector2D{0.0f, 150.0f}, Vector2D{280.0f, 0.0f}, Vector2D{0.0f, 0.0f}})
    {
      Entity EnemyEntity = Context.World.CreateEntity();
      Context.World.AddComponent<TransformComponent>(EnemyEntity, {SpawnPos, 0.0f, {1.0f, 1.0f}});
      Context.World.AddComponent<VelocityComponent>(EnemyEntity, {});
      Context.World.AddComponent<SpriteComponent>(EnemyEntity, {Context.Assets.LoadTexture(PlayerTexturePath), Engine::Rect{0.0f, 0.0f, 16.0f, 16.0f}});
      Context.World.AddComponent<FlowFieldFollowComponent>(EnemyEntity, FlowFieldFollowComponent{0, 30.0f});
    }

    BuildToolbar(Context);

#ifdef ENGINE_WITH_DEBUG_UI
    // Points Game.exe's own linked ImGui copy at Engine.dll's context (see DebugOverlay.h).
    ImGui::SetCurrentContext(Context.Overlay.GetImGuiContext());
#endif

    return true;
  }

  void PreUpdate(EngineContext &Context)
  {
    if (Context.Input.IsMouseButtonJustPressed(1)) // 1 = left mouse button
    {
      Vector2D MousePos{
          static_cast<float>(Context.Input.GetMouseX()),
          static_cast<float>(Context.Input.GetMouseY())};
      PendingMoveCommands.push_back(MoveCommand{Context.MainCamera.ScreenToWorld(MousePos)});
    }
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

    // While, not if: lag can queue more than one click per frame.
    while (!PendingMoveCommands.empty())
    {
      Vector2D ClickWorldPos = PendingMoveCommands.front().TargetWorldPos;
      PendingMoveCommands.pop_front();

      // Local prediction first, always, so offline/hosting/client clicks share one code path.
      uint32_t LocalNetworkId = Context.World.GetResource<LocalPlayerState>()->NetworkId;
      std::optional<Entity> LocalPlayer = FindEntityByNetworkId(Context.World, LocalNetworkId);
      if (LocalPlayer)
      {
        ApplyLocalPathToEntity(Context, *LocalPlayer, ClickWorldPos);
      }

      if (Context.Network.IsHost())
      {
        BroadcastSetPathFor(Context, LocalNetworkId);
      }
      else if (Context.Network.GetHostConnectionId() != 0)
      {
        ServerMovePlayer(Context, ClickWorldPos);
      }
    }
  }

  void PostUpdate(EngineContext &Context, float DeltaTime)
  {
    UpdateCameraFollow(Context, DeltaTime);
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
