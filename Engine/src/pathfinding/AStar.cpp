#include "AStar.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace Engine
{
  namespace
  {
    struct OpenNode
    {
      float FScore;
      int32_t CellIndex;
    };

    struct OpenNodeGreater
    {
      bool operator()(const OpenNode &A, const OpenNode &B) const
      {
        return A.FScore > B.FScore;
      }
    };

    float Heuristic(int32_t FromX, int32_t FromY, int32_t ToX, int32_t ToY, NavMovement Movement)
    {
      float DeltaX = static_cast<float>(std::abs(ToX - FromX));
      float DeltaY = static_cast<float>(std::abs(ToY - FromY));

      if (Movement == NavMovement::FourDirectional)
      {
        return DeltaX + DeltaY;
      }

      // Octile distance: admissible for a cost model where diagonal steps cost sqrt(2) and
      // orthogonal steps cost 1. Chebyshev (max(DeltaX, DeltaY)) is only admissible when
      // diagonal cost equals orthogonal cost, which isn't the case here.
      constexpr float SqrtTwoMinusOne = 0.41421356f;
      return (DeltaX + DeltaY) + SqrtTwoMinusOne * std::min(DeltaX, DeltaY);
    }

    // Shared setup: converts Start/Goal to cell coords, fails if either is entirely outside the
    // grid, there's nothing a search could ever reach from/to there.
    bool TryGetSearchCells(const NavGrid &Grid, Vector2D Start, Vector2D Goal,
                            int32_t &StartX, int32_t &StartY, int32_t &GoalX, int32_t &GoalY)
    {
      StartX = Grid.WorldToCellX(Start.X);
      StartY = Grid.WorldToCellY(Start.Y);
      GoalX = Grid.WorldToCellX(Goal.X);
      GoalY = Grid.WorldToCellY(Goal.Y);

      return Grid.IsInBounds(StartX, StartY) && Grid.IsInBounds(GoalX, GoalY);
    }
  }

  std::vector<Vector2D> FindPath(const NavGrid &Grid, Vector2D Start, Vector2D Goal, NavMovement Movement)
  {
    int32_t StartX = 0, StartY = 0, GoalX = 0, GoalY = 0;
    if (!TryGetSearchCells(Grid, Start, Goal, StartX, StartY, GoalX, GoalY))
    {
      return {};
    }

    size_t CellCount = static_cast<size_t>(Grid.Width) * static_cast<size_t>(Grid.Height);
    std::vector<float> GScore(CellCount, std::numeric_limits<float>::infinity());
    std::vector<int32_t> CameFrom(CellCount, -1);
    std::vector<bool> Visited(CellCount, false);

    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeGreater> Open;

    int32_t StartIndex = Grid.CellIndex(StartX, StartY);
    int32_t GoalIndex = Grid.CellIndex(GoalX, GoalY);

    GScore[static_cast<size_t>(StartIndex)] = 0.0f;
    Open.push(OpenNode{Heuristic(StartX, StartY, GoalX, GoalY, Movement), StartIndex});

    while (!Open.empty())
    {
      OpenNode Current = Open.top();
      Open.pop();

      if (Visited[static_cast<size_t>(Current.CellIndex)])
      {
        continue;
      }
      Visited[static_cast<size_t>(Current.CellIndex)] = true;

      if (Current.CellIndex == GoalIndex)
      {
        break;
      }

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

                         float TentativeGScore = GScore[static_cast<size_t>(Current.CellIndex)] + StepCost;
                         if (TentativeGScore < GScore[static_cast<size_t>(NeighborIndex)])
                         {
                           GScore[static_cast<size_t>(NeighborIndex)] = TentativeGScore;
                           CameFrom[static_cast<size_t>(NeighborIndex)] = Current.CellIndex;
                           float FScore = TentativeGScore + Heuristic(NeighborX, NeighborY, GoalX, GoalY, Movement);
                           Open.push(OpenNode{FScore, NeighborIndex});
                         }
                       });
    }

    if (GScore[static_cast<size_t>(GoalIndex)] == std::numeric_limits<float>::infinity())
    {
      return {};
    }

    std::vector<Vector2D> RawWaypoints;
    int32_t Cursor = GoalIndex;
    while (Cursor != StartIndex)
    {
      int32_t CellX = Cursor % Grid.Width;
      int32_t CellY = Cursor / Grid.Width;
      RawWaypoints.push_back(Grid.CellToWorldCenter(CellX, CellY));
      Cursor = CameFrom[static_cast<size_t>(Cursor)];
    }
    std::reverse(RawWaypoints.begin(), RawWaypoints.end());

    // Collapse collinear interior points (identical incoming/outgoing direction) to their two
    // endpoints; no full string-pulling/Theta* smoothing, just dropping redundant midpoints on
    // an already-straight run.
    std::vector<Vector2D> Waypoints;
    for (size_t Index = 0; Index < RawWaypoints.size(); ++Index)
    {
      if (Index > 0 && Index + 1 < RawWaypoints.size())
      {
        Vector2D Incoming = (RawWaypoints[Index] - RawWaypoints[Index - 1]).Normalized();
        Vector2D Outgoing = (RawWaypoints[Index + 1] - RawWaypoints[Index]).Normalized();
        if (Incoming.DistanceSquared(Outgoing) < 0.0001f)
        {
          continue;
        }
      }
      Waypoints.push_back(RawWaypoints[Index]);
    }

    return Waypoints;
  }

  bool HasPath(const NavGrid &Grid, Vector2D Start, Vector2D Goal, NavMovement Movement)
  {
    int32_t StartX = 0, StartY = 0, GoalX = 0, GoalY = 0;
    if (!TryGetSearchCells(Grid, Start, Goal, StartX, StartY, GoalX, GoalY))
    {
      return false;
    }

    int32_t StartIndex = Grid.CellIndex(StartX, StartY);
    int32_t GoalIndex = Grid.CellIndex(GoalX, GoalY);

    if (StartIndex == GoalIndex)
    {
      return true;
    }

    size_t CellCount = static_cast<size_t>(Grid.Width) * static_cast<size_t>(Grid.Height);
    std::vector<bool> Visited(CellCount, false);
    std::queue<int32_t> Frontier;

    Visited[static_cast<size_t>(StartIndex)] = true;
    Frontier.push(StartIndex);

    while (!Frontier.empty())
    {
      int32_t Current = Frontier.front();
      Frontier.pop();

      int32_t CurrentX = Current % Grid.Width;
      int32_t CurrentY = Current / Grid.Width;

      bool Found = false;
      ForEachNeighbor(Grid, CurrentX, CurrentY, Movement,
                       [&](int32_t NeighborX, int32_t NeighborY, float)
                       {
                         int32_t NeighborIndex = Grid.CellIndex(NeighborX, NeighborY);
                         if (Visited[static_cast<size_t>(NeighborIndex)])
                         {
                           return;
                         }
                         Visited[static_cast<size_t>(NeighborIndex)] = true;
                         if (NeighborIndex == GoalIndex)
                         {
                           Found = true;
                         }
                         Frontier.push(NeighborIndex);
                       });

      if (Found)
      {
        return true;
      }
    }

    return false;
  }
}
