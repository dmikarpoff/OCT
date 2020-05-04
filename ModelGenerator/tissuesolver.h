#pragma once

#include "layer_property.h"

// banded matrix + Choletsky decomposition
class TissueDirectSolver
{
 public:
    explicit TissueDirectSolver(const LayerProperty& lp, int width, int height);
private:
    uint64_t LinearIndex(uint32_t i, uint32_t j)
    {
        return i * height_ + j;
    }

    uint32_t width_ = 0;
    uint32_t height_ = 0;
};
