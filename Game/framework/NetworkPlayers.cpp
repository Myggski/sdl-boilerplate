#include "NetworkPlayers.h"
#include "AssetPaths.h"
#include "CollisionSettings.h"
#include "src/Log.h"
#include "src/pathfinding/NavGrid.h"
#include "src/pathfinding/AStar.h"
#include "src/ecs/PrefabRegistry.h"
#include "src/ecs/component/NetworkIdComponent.h"
#include "src/ecs/component/TransformComponent.h"
#include "src/ecs/component/VelocityComponent.h"
#include "src/ecs/component/SpriteComponent.h"
#include "src/ecs/component/AnimationComponent.h"
#include "src/ecs/component/ColliderComponent.h"
#include "src/ecs/component/PathFollowComponent.h"

namespace Game
{
  using namespace Engine;

  Entity SpawnPlayerEntity(EngineContext &Context)
  {
    Entity NewEntity = Context.World.CreateEntity();
    // Default (0, 0) - the replicated snapshot fills in the real Position right after.
    Context.World.AddComponent<TransformComponent>(NewEntity, {});
    Context.World.AddComponent<VelocityComponent>(NewEntity, {0.f, 0.f});
    Context.World.AddComponent<SpriteComponent>(NewEntity, {Context.Assets.LoadTexture(PlayerTexturePath), Engine::Rect{0.0f, 0.0f, 16.0f, 16.0f}});
    Context.World.AddComponent<AnimationComponent>(
        NewEntity, {MakeGridFrames(Vector2D{0.0f, 0.0f}, 16.0f, 16.0f, 4u), 0.15f});
    Context.World.AddComponent<ColliderComponent>(
        NewEntity, ColliderComponent{8.0f, Layers::Player});
    return NewEntity;
  }

  REGISTER_PREFAB(Player, PlayerPrefabId, SpawnPlayerEntity)

  void ApplyLocalPathToEntity(EngineContext &Context, Entity Target, Vector2D ClickWorldPos)
  {
    NavGrid *Grid = Context.World.GetResource<NavGrid>();
    TransformComponent *Transform = Context.World.GetComponent<TransformComponent>(Target);
    if (!Grid || !Transform)
    {
      return;
    }

    NetworkIdComponent *NetId = Context.World.GetComponent<NetworkIdComponent>(Target);
    uint32_t NetworkId = NetId ? NetId->NetworkId : 0;

    std::vector<Vector2D> Waypoints = FindPath(*Grid, Transform->Position, ClickWorldPos);
    if (!Waypoints.empty())
    {
      size_t WaypointCount = Waypoints.size(); // read before the move below leaves Waypoints empty
      Context.World.AddComponent<PathFollowComponent>(Target, PathFollowComponent{std::move(Waypoints)});
      ENGINE_LOG_INFO("Path found to (%.0f, %.0f) for network id %u: %zu waypoints",
                       ClickWorldPos.X, ClickWorldPos.Y, NetworkId, WaypointCount);
    }
    else
    {
      // Empty isn't necessarily a bug - a blocked/already-occupied cell also produces no path.
      int32_t ClickCellX = Grid->WorldToCellX(ClickWorldPos.X);
      int32_t ClickCellY = Grid->WorldToCellY(ClickWorldPos.Y);
      bool InBounds = Grid->IsInBounds(ClickCellX, ClickCellY);
      ENGINE_LOG_INFO("No path to (%.0f, %.0f) [cell %d,%d] for network id %u: %s",
                       ClickWorldPos.X, ClickWorldPos.Y, ClickCellX, ClickCellY, NetworkId,
                       !InBounds ? "out of grid bounds" : (Grid->IsWalkable(ClickCellX, ClickCellY) ? "unreachable (isolated from player)" : "clicked cell is blocked"));
    }
  }
}
