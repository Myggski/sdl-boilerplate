#pragma once

#include "Core.h"
#include <cstdint>
#include <initializer_list>
#include <utility>

namespace Engine
{
  // Symmetric layer-vs-layer interaction table: declare, once per PAIR of layers, whether
  // entities on those layers should ever be checked against each other. ColliderComponent only
  // ever specifies its own Layer, never a mask. CollisionSystem consults this instead.
  // SetLayersCollide is symmetric by construction (you cannot declare A-collides-with-B without
  // B-collides-with-A), so there is exactly one place to edit a relationship and no way for the
  // two sides to drift out of sync.
  //
  // Game-owned, not Engine-owned: layers are inherently game-specific, so a game declares its own
  // CollisionMatrix, typically once, e.g.
  //   CollisionMatrix Matrix({
  //       {Layers::Player, Layers::Terrain},
  //       {Layers::Player, Layers::Prop},
  //   });
  class ENGINE_API CollisionMatrix
  {
  public:
    CollisionMatrix();
    // Convenience: populate from a flat list of layer pairs in one call, so the whole matrix can
    // be declared in one place, Theme.h-style, rather than a sequence of SetLayersCollide calls.
    CollisionMatrix(std::initializer_list<std::pair<uint32_t, uint32_t>> Pairs);

    void SetLayersCollide(uint32_t LayerA, uint32_t LayerB, bool ShouldCollide = true);
    bool DoLayersCollide(uint32_t LayerA, uint32_t LayerB) const;

  private:
    // One derived mask per layer bit (up to 32 distinct layers, matching uint32_t's width):
    // LayerMasks[N] is the OR of every layer bit that layer N has been told it collides with.
    uint32_t LayerMasks[32] = {};
  };
}
