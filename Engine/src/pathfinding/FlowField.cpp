#include "FlowField.h"
#include <cmath>
#include <limits>
#include <queue>

namespace Engine
{
  bool FlowField::IsInBounds(int32_t CellX, int32_t CellY) const
  {
    return CellX >= 0 && CellX < Width && CellY >= 0 && CellY < Height;
  }

  int32_t FlowField::CellIndex(int32_t CellX, int32_t CellY) const
  {
    return CellY * Width + CellX;
  }

  int32_t FlowField::WorldToCellX(float WorldX) const
  {
    return static_cast<int32_t>(std::floor((WorldX - Origin.X) / CellSize));
  }

  int32_t FlowField::WorldToCellY(float WorldY) const
  {
    return static_cast<int32_t>(std::floor((WorldY - Origin.Y) / CellSize));
  }

  namespace
  {
    struct OpenNode
    {
      float Cost;
      int32_t CellIndex;
    };

    struct OpenNodeGreater
    {
      bool operator()(const OpenNode &A, const OpenNode &B) const
      {
        return A.Cost > B.Cost;
      }
    };
  }

  FlowField BuildFlowField(const NavGrid &Grid, Vector2D Goal, NavMovement Movement)
  {
    FlowField Field;
    Field.Origin = Grid.Origin;
    Field.CellSize = Grid.CellSize;
    Field.Width = Grid.Width;
    Field.Height = Grid.Height;

    size_t CellCount = static_cast<size_t>(Grid.Width) * static_cast<size_t>(Grid.Height);
    Field.Cost.assign(CellCount, std::numeric_limits<float>::infinity());
    Field.Directions.assign(CellCount, Vector2D{0.0f, 0.0f});

    int32_t GoalX = Grid.WorldToCellX(Goal.X);
    int32_t GoalY = Grid.WorldToCellY(Goal.Y);

    if (!Grid.IsInBounds(GoalX, GoalY))
    {
      // Goal is entirely outside the grid: nothing reachable, Cost stays all-infinity,
      // Directions stay {0,0} everywhere.
      return Field;
    }

    int32_t GoalIndex = Grid.CellIndex(GoalX, GoalY);

    // Integration pass: Dijkstra seeded at the goal cell. Uniform-cost BFS is Dijkstra's special
    // case, so one code path handles both movement modes, a plain BFS would get EightDirectional
    // wrong since diagonal steps cost more than orthogonal ones there.
    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeGreater> Open;
    std::vector<bool> Visited(CellCount, false);

    Field.Cost[static_cast<size_t>(GoalIndex)] = 0.0f;
    Open.push(OpenNode{0.0f, GoalIndex});

    while (!Open.empty())
    {
      OpenNode Current = Open.top();
      Open.pop();

      if (Visited[static_cast<size_t>(Current.CellIndex)])
      {
        continue;
      }
      Visited[static_cast<size_t>(Current.CellIndex)] = true;

      int32_t CurrentX = Current.CellIndex % Grid.Width;
      int32_t CurrentY = Current.CellIndex / Grid.Width;

      ForEachNeighbor(Grid, CurrentX, CurrentY, Movement,
                       [&](int32_t NeighborX, int32_t NeighborY, float StepCost)
                       {
                         int32_t NeighborIndex = Grid.CellIndex(NeighborX, NeighborY);
                         if (Visited[static_cast<size_t>(NeighborIndex)])
                         {
                           return;
                         }

                         float TentativeCost = Field.Cost[static_cast<size_t>(Current.CellIndex)] + StepCost;
                         if (TentativeCost < Field.Cost[static_cast<size_t>(NeighborIndex)])
                         {
                           Field.Cost[static_cast<size_t>(NeighborIndex)] = TentativeCost;
                           Open.push(OpenNode{TentativeCost, NeighborIndex});
                         }
                       });
    }

    // Flow pass: every walkable cell points toward whichever neighbor has the lowest finite
    // Cost. Normalized() here (a runtime sqrt) rather than a lookup table of 8 constant unit
    // vectors: this runs once per goal build, not per agent per frame, the difference is not
    // meaningful at that rate.
    for (int32_t CellY = 0; CellY < Grid.Height; ++CellY)
    {
      for (int32_t CellX = 0; CellX < Grid.Width; ++CellX)
      {
        if (!Grid.IsWalkable(CellX, CellY))
        {
          continue;
        }

        int32_t CellIdx = Grid.CellIndex(CellX, CellY);
        float BestCost = Field.Cost[static_cast<size_t>(CellIdx)];
        Vector2D BestDirection{0.0f, 0.0f};

        ForEachNeighbor(Grid, CellX, CellY, Movement,
                         [&](int32_t NeighborX, int32_t NeighborY, float)
                         {
                           int32_t NeighborIndex = Grid.CellIndex(NeighborX, NeighborY);
                           float NeighborCost = Field.Cost[static_cast<size_t>(NeighborIndex)];
                           if (NeighborCost < BestCost)
                           {
                             BestCost = NeighborCost;
                             Vector2D Delta{static_cast<float>(NeighborX - CellX), static_cast<float>(NeighborY - CellY)};
                             BestDirection = Delta.Normalized();
                           }
                         });

        Field.Directions[static_cast<size_t>(CellIdx)] = BestDirection;
      }
    }

    return Field;
  }
}
