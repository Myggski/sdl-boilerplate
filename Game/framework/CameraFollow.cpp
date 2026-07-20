#include "CameraFollow.h"
#include "CameraSettings.h"
#include "src/ecs/component/NetworkIdComponent.h"
#include "src/ecs/component/TransformComponent.h"
#include <cmath>
#include <vector>

namespace Game
{
  using namespace Engine;

  void UpdateCameraFollow(EngineContext &Context, float DeltaTime)
  {
    std::vector<Vector2D> PlayerPositions;
    Context.World.ForEach<NetworkIdComponent, TransformComponent>(
        [&](Entity, NetworkIdComponent &, TransformComponent &Transform)
        { PlayerPositions.push_back(Transform.Position); });

    if (PlayerPositions.empty())
    {
      return;
    }

    CameraFitResult Fit = Context.MainCamera.ComputeFitToTargets(
        PlayerPositions, CameraTuning::MinZoom, CameraTuning::MaxZoom, CameraTuning::Padding);

    // Static: persists across frames, snapping to the first fit on the very first call instead of
    // animating in from {0, 0}.
    static Vector2D FocusCenter = Fit.Center;
    static float FocusZoom = Fit.Zoom;

    float LerpFactor = 1.0f - std::exp(-CameraTuning::FollowSmoothing * DeltaTime);
    FocusCenter = FocusCenter + (Fit.Center - FocusCenter) * LerpFactor;
    FocusZoom = FocusZoom + (Fit.Zoom - FocusZoom) * LerpFactor;

    Context.MainCamera.CenterOn(FocusCenter);
    Context.MainCamera.SetZoom(FocusZoom);
  }
}
