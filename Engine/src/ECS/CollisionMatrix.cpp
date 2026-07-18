#include "CollisionMatrix.h"

namespace Engine
{
  namespace
  {
    // Bit index (0-31) of a single-bit layer value, or 32 if Layer isn't a single bit (e.g. 0, or
    // an already-OR'd combination). Callers are expected to pass single-bit layers, same as
    // ColliderComponent::Layer.
    uint32_t BitIndex(uint32_t Layer)
    {
      for (uint32_t Index = 0; Index < 32; ++Index)
      {
        if (Layer == (1u << Index))
        {
          return Index;
        }
      }
      return 32;
    }
  }

  CollisionMatrix::CollisionMatrix() = default;

  CollisionMatrix::CollisionMatrix(std::initializer_list<std::pair<uint32_t, uint32_t>> Pairs)
  {
    for (const std::pair<uint32_t, uint32_t> &LayerPair : Pairs)
    {
      SetLayersCollide(LayerPair.first, LayerPair.second);
    }
  }

  void CollisionMatrix::SetLayersCollide(uint32_t LayerA, uint32_t LayerB, bool ShouldCollide)
  {
    uint32_t IndexA = BitIndex(LayerA);
    uint32_t IndexB = BitIndex(LayerB);
    if (IndexA >= 32 || IndexB >= 32)
    {
      return;
    }

    if (ShouldCollide)
    {
      LayerMasks[IndexA] |= LayerB;
      LayerMasks[IndexB] |= LayerA;
    }
    else
    {
      LayerMasks[IndexA] &= ~LayerB;
      LayerMasks[IndexB] &= ~LayerA;
    }
  }

  bool CollisionMatrix::DoLayersCollide(uint32_t LayerA, uint32_t LayerB) const
  {
    uint32_t IndexA = BitIndex(LayerA);
    if (IndexA >= 32)
    {
      return false;
    }

    return (LayerMasks[IndexA] & LayerB) != 0;
  }
}
