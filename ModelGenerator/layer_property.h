#pragma once

#include <algorithm>
#include <cmath>

struct LayerProperty
{
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;
    double alpha = 0.5;
    double sigma = 1.0;
    int max_z = 1;
    int border_aver_window = 1;
    double border_alpha = 0.5;
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
