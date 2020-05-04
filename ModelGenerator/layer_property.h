#pragma once

#include <algorithm>
#include <cmath>

struct LayerProperty
{
    double a;
    double b;
    double c;
    double d;
    double alpha;
    double sigma;
    int max_z;
    double border_alpha;
};

inline LayerProperty Normalize(const LayerProperty& lp)
{
    static constexpr double kNormalizationTolerance = 1e-8;

    LayerProperty res = lp;
    res.a = std::abs(lp.a);
    res.b = std::abs(lp.b);
    res.c = std::abs(lp.c);
    res.d = std::abs(lp.d);
    double sum = res.a + res.b + res.c + res.d;
    if (std::abs(sum - 1.0) > kNormalizationTolerance)
    {
        if (sum < 1E-8)
        {
            res.a = res.b = res.c = res.d = 0.25;
        }
        else
        {
            res.a /= sum;
            res.b /= sum;
            res.c /= sum;
            res.d /= sum;
        }
    }
    res.alpha = std::min(1.0, std::max(res.alpha, 0.0));
    return res;
}
