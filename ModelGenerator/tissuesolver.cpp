#include "tissuesolver.h"

#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/banded.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <random>

TissueDirectSolver::TissueDirectSolver(const LayerProperty& lp, int width, int height)
    : width_(width)
    , height_(height)
{
    size_t band_width = std::max(width_, height_);
    boost::numeric::ublas::banded_matrix<double> A(width * height, width * height, band_width, band_width);
    for (uint32_t i = 0; i < width_; ++i)
    {
        for (uint32_t j = 0; j < height_; ++j)
        {
            uint64_t idx = LinearIndex(i, j);
            A(idx, idx) = 1.0;
            if (i > 0)
            {
                A(idx, LinearIndex(i - 1, j)) = lp.a;
            }
            if (j > 0)
            {
                A(idx, LinearIndex(i, j - 1)) = lp.d;
            }
            if (j + 1 < height_)
            {
                A(idx, LinearIndex(i, j + 1)) = lp.c;
            }
            if (i + 1 < width_)
            {
                A(idx, LinearIndex(i + 1, j)) = lp.b;
            }
        }
    }
    std::mt19937 generator(std::random_device{}());
    std::normal_distribution<> noise;
    boost::numeric::ublas::vector<double> w(A.size1());
    for (size_t i = 0; i < w.size(); ++i)
        w(i) = noise(generator);

    boost::numeric::ublas::permutation_matrix<> pm(A.size1());
    boost::numeric::ublas::lu_factorize(A, pm);
    boost::numeric::ublas::lu_substitute(A, pm, w);
    std::cout << w << std::endl;
}
