
#include "kalman_filter.h"

#include <oct_utils/matrix_view.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace oct {

namespace {

using vec_double = std::vector<double>;
using vec_vec_double = std::vector<vec_double>;

}  // namespace

ScanData KalmanFilter::applyKalmanFiltering(const ScanData& src_data) const
{
    auto width = src_data.width();
    auto height = src_data.height();
    vec_vec_double s_0(height, vec_double(width, 0.0));
    vec_double raw_s_m(width * height, 0.0);
    MatrixView<vec_double> s_m(raw_s_m, width);
    vec_vec_double phi(height, vec_double(width, 0.0));
    vec_vec_double u_h(height, vec_double(width, 0.0));
    vec_vec_double u_v(height, vec_double(width, 0.0));

    boost::numeric::ublas::matrix<double> A(5, 10);
    A(0, 0) = A(0, 1) = 0.5;
    A(1, 2) = A(1, 3) = 0.5;
    A(2, 4) = A(2, 5) = 0.5;
    A(2, 7) = A(2, 8) = M_PI;
    A(3, 6) = A(3, 7) = 0.5;
    A(4, 8) = A(4, 9) = 0.5;

    for (size_t h = 0; h < height; ++h)
    {
        for (size_t w = 0; w < width; ++w)
        {
            double divider = 0.0;
            if (h > 0)
            {
                s_0[h][w] += s_0[h - 1][w];
                s_m(h, w) += s_m(h - 1, w);
                phi[h][w] += phi[h - 1][w] + 2 * M_PI * u_v[h - 1][w];
                u_h[h][w] += u_h[h - 1][w];
                u_v[h][w] += u_v[h - 1][w];
                divider += 1.0;
            }
            if (w > 0)
            {
                s_0[h][w] += s_0[h][w - 1];
                s_m(h, w) += s_m(h, w - 1);
                phi[h][w] += phi[h][w - 1] + 2 * M_PI * u_h[h][w - 1];
                u_h[h][w] += u_h[h][w - 1];
                u_v[h][w] += u_v[h][w - 1];
                divider += 1.0;
            }
            if (h != 0 || w != 0)
            {
                s_0[h][w] /= divider;
                s_m(h, w) /= divider;
                phi[h][w] /= divider;
                u_h[h][w] /= divider;
                u_v[h][w] /= divider;
            }

            boost::numeric::ublas::vector<double> prediction(4);
            prediction(0) = s_0[h][w] + s_m(h, w) * cos(phi[h][w]);
            prediction(1) = s_0[h][w] + s_m(h, w) * cos(phi[h][w] + 2 * M_PI * u_h[h][w]);
            prediction(2) = s_0[h][w] + s_m(h, w) * cos(phi[h][w] + 2 * M_PI * u_v[h][w]);
            prediction(3) = s_0[h][w] + s_m(h, w) * cos(phi[h][w] + 2 * M_PI * u_h[h][w] +
                                                        2 * M_PI * u_v[h][w]);
            auto expected = prediction;
            expected(0) = src_data(h, w);
            if (w + 1 < width)
            {
                expected(1) = src_data(h, w + 1);
            }
            if (h + 1 < height)
            {
                expected(2) = src_data(h + 1, w);
            }
            if (w + 1 < width && h + 1 < height)
            {
                expected(3) = src_data(h + 1, w + 1);
            }
            auto v = expected - prediction;
            boost::numeric::ublas::matrix<double> H(5, 4);
            for (int i = 0; i < 4; ++i)
                H(0, i) = 1.0;
            H(1, 0) = cos(phi[h][w]);
            H(1, 1) = cos(phi[h][w] + 2.0 * M_PI * u_h[h][w]);
            H(1, 2) = cos(phi[h][w] + 2.0 * M_PI * u_v[h][w]);
            H(1, 3) = cos(phi[h][w] + 2.0 * M_PI * u_h[h][w] + 2.0 * M_PI * u_v[h][w]);
            H(2, 0) = -s_m(h, w) * sin(phi[h][w]);
            H(2, 1) = -s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_h[h][w]);
            H(2, 2) = -s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_v[h][w]);
            H(2, 3) = -s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_h[h][w] + 2.0 * M_PI * u_v[h][w]);
            H(3, 0) = 0.0;
            H(3, 1) = -2.0 * M_PI * s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_h[h][w]);
            H(3, 2) = 0.0;
            H(3, 3) = -2.0 * M_PI * s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_h[h][w] + 2.0 * M_PI * u_v[h][w]);
            H(4, 0) = 0.0;
            H(4, 1) = 0.0;
            H(4, 2) = -2.0 * M_PI * s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_v[h][w]);
            H(4, 3) = -2.0 * M_PI * s_m(h, w) * sin(phi[h][w] + 2.0 * M_PI * u_h[h][w] + 2.0 * M_PI * u_v[h][w]);
        }
    }

    return ScanData(width, height, std::move(raw_s_m));
}

}  // namespace oct
