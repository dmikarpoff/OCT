
#include "kalman_filter.h"

#include <oct_utils/matrix_view.h>

#include <boost/log/trivial.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <type_traits>
#include <vector>
#include <unordered_map>

namespace {

template <typename T>
boost::numeric::ublas::matrix<T> inverse(boost::numeric::ublas::matrix<T> input)
{
    boost::numeric::ublas::permutation_matrix<size_t> pm(input.size1());
    int res = boost::numeric::ublas::lu_factorize(input, pm);
    if (res != 0)
    {
        throw std::domain_error("Failed to inverse matrix with code " + std::to_string(res));
    }
    boost::numeric::ublas::matrix<T> inverted = boost::numeric::ublas::identity_matrix<T>(input.size1());
    boost::numeric::ublas::lu_substitute(input, pm, inverted);
    return inverted;
}

constexpr double kReadWriteFrac = 0.1;
constexpr const char kFilteringStatus[] = "Filtering data ...";

using matrix_t = oct::KalmanFilter::matrix_t;
using ublas_vec = boost::numeric::ublas::vector<double>;
template <typename T> using std_vec = std::vector<T>;

struct CovarianceIdx {
    std::pair<uint, uint> left_idx;
    std::pair<uint, uint> right_idx;

    struct CovarianceIdxHasher {
        size_t operator()(const CovarianceIdx& idx) const {
            return std::hash<uint>()(idx.left_idx.first) ^ std::hash<uint>()(idx.left_idx.second) ^
                   std::hash<uint>()(idx.right_idx.first) ^ std::hash<uint>()(idx.right_idx.second);
        }
    };
};

bool operator==(const CovarianceIdx& lhs, const CovarianceIdx& rhs) {
    return lhs.left_idx == rhs.left_idx && lhs.right_idx == rhs.right_idx;
}

struct ExtendedEstimatorState {
    std_vec<ublas_vec> prev_signal_param;
    std_vec<ublas_vec> cur_signal_param;
    std_vec<boost::numeric::ublas::matrix<double>> prev_r;
    std_vec<boost::numeric::ublas::matrix<double>> next_r;
    std::unordered_map<CovarianceIdx, matrix_t, CovarianceIdx::CovarianceIdxHasher> error_cov;
};

struct ReducedEstimatorState {
    std_vec<ublas_vec> prev_signal_param;
    std_vec<ublas_vec> cur_signal_param;
};

template <bool kAuto>
using EstimatorState = std::conditional_t<kAuto, ExtendedEstimatorState, ReducedEstimatorState>;

matrix_t estimateRpr(std::false_type, const ReducedEstimatorState& /*reduced_state*/,
                     const CovarianceIdx& /*idx*/, const matrix_t& /*A*/, const matrix_t& /*B*/,
                     const matrix_t& R_pr) {
    return R_pr;
}

std::pair<matrix_t, bool> getMatrixFromCache(const ExtendedEstimatorState& state, CovarianceIdx idx) {
    bool transpose = false;
    if (idx.left_idx > idx.right_idx) {
        std::swap(idx.left_idx, idx.right_idx);
        transpose = true;
    }
    auto it = state.error_cov.find(idx);
    if (it != state.error_cov.end()) {
        return {transpose ? boost::numeric::ublas::trans(it->second) : it->second, true};
    }
    return {matrix_t{}, false};
}

template <typename T>
std::pair<T, T> decreaseFirst(const std::pair<T, T>& p) {
    return {p.first - 1, p.second};
}

template <typename T>
std::pair<T, T> decreaseSecond(const std::pair<T, T>& p) {
    return {p.first, p.second - 1};
}

matrix_t estimateRpr(std::true_type, ExtendedEstimatorState& state,
                     const CovarianceIdx& idx, const matrix_t& A, const matrix_t& B,
                     const matrix_t& R_w) {
    {
        auto m = getMatrixFromCache(state, idx);
        if (m.second) {
            return m.first;
        }
    }
    auto res = R_w;
    auto estimate_or_extract = [&](const CovarianceIdx& idx) {
        auto m = getMatrixFromCache(state, idx);
        if (!m.second) {
            m.first = estimateRpr(std::true_type{}, state, idx, A, B, R_w);
            state.error_cov[idx] = m.first;
        }
        return m.first;
    };

    if (idx.left_idx.first > 0 && idx.right_idx.first > 0) {
        auto l = decreaseFirst(idx.left_idx);
        auto r = decreaseFirst(idx.right_idx);
        auto m = estimate_or_extract({l, r});
        matrix_t left_prod = boost::numeric::ublas::prod(A, m);
        res = res + boost::numeric::ublas::prod(left_prod, boost::numeric::ublas::trans(A));
    }

    if (idx.left_idx.first > 0 && idx.right_idx.second > 0) {
        auto l = decreaseFirst(idx.left_idx);
        auto r = decreaseSecond(idx.right_idx);
        auto m = estimate_or_extract({l, r});
        matrix_t left_prod = boost::numeric::ublas::prod(A, m);
        res = res + boost::numeric::ublas::prod(left_prod, boost::numeric::ublas::trans(B));
    }

    if (idx.left_idx.second > 0 && idx.right_idx.second > 0) {
        auto l = decreaseSecond(idx.left_idx);
        auto r = decreaseSecond(idx.right_idx);
        auto m = estimate_or_extract({l, r});
        matrix_t left_prod = boost::numeric::ublas::prod(B, m);
        res = res + boost::numeric::ublas::prod(left_prod, boost::numeric::ublas::trans(B));
    }
    if (idx.left_idx.second > 0 && idx.right_idx.first > 0) {
        auto l = decreaseSecond(idx.left_idx);
        auto r = decreaseFirst(idx.right_idx);
        auto m = estimate_or_extract({l, r});
        matrix_t left_prod = boost::numeric::ublas::prod(B, m);
        res = res + boost::numeric::ublas::prod(left_prod, boost::numeric::ublas::trans(A));
    }

    return res;
}

}  // namespace

namespace oct
{

void KalmanFilter::filter(QString input_csv, QString output_csv, QString output_png,
                          boost::numeric::ublas::matrix<double> R_pr,
                          boost::numeric::ublas::matrix<double> R_n)
{
    R_pr_ = std::move(R_pr);
    R_n_ = std::move(R_n);
    auto_r_pr_ = false;
    filterImpl(input_csv, output_csv, output_png);
}

void KalmanFilter::filterAuto(QString input_csv, QString output_csv, QString output_png,
                              boost::numeric::ublas::matrix<double> R_w,
                              boost::numeric::ublas::matrix<double> R_n) {
    R_w_ = std::move(R_w);
    R_n_ = std::move(R_n);
    auto_r_pr_ = true;
    filterImpl(input_csv, output_csv, output_png);
}

template <bool kAutoFilter>
ScanData KalmanFilter::applyKalmanFiltering(const ScanData& src_data) const
{
    auto width = src_data.width();
    auto height = src_data.height();
    std::vector<double> raw_data(width * height, 0.0);

    boost::numeric::ublas::matrix<double> A(5, 5, 0.0);
    boost::numeric::ublas::matrix<double> B(5, 5, 0.0);
    A(0, 0) = B(0, 0) = 0.5;
    A(1, 1) = B(1, 1) = 0.5;
    A(2, 2) = B(2, 2) = 0.5;
    A(2, 4) = B(2, 3) = M_PI;
    A(3, 3) = B(3, 3) = 0.5;
    A(4, 4) = B(4, 4) = 0.5;

    EstimatorState<kAutoFilter> state;
    state.prev_signal_param.assign(width, ublas_vec(5, 0.0));

    for (size_t h = 0; h < height; ++h)
    {
        state.cur_signal_param.assign(width, ublas_vec(5, 0.0));
        for (size_t w = 0; w < width; ++w)
        {
            double progress = static_cast<double>(h * width + w) / (width * height) *
                    (1.0 - 2.0 * kReadWriteFrac) + kReadWriteFrac;
            emit reportProgress(kFilteringStatus, progress);
            if (interrupted_)
            {
                return {width, height, {}};
            }
            if (h > 0)
            {
                state.cur_signal_param[w] += boost::numeric::ublas::prod(A, state.prev_signal_param[w]);
            }
            if (w > 0)
            {
                state.cur_signal_param[w] += boost::numeric::ublas::prod(B, state.cur_signal_param[w - 1]);
            }

            const double s_0 = state.cur_signal_param[w](0);
            const double s_m = state.cur_signal_param[w](1);
            const double phi = state.cur_signal_param[w](2);
            const double u_h = state.cur_signal_param[w](3);
            const double u_v = state.cur_signal_param[w](4);

            ublas_vec prediction(4);
            prediction(0) = s_0 + s_m * cos(phi);
            prediction(1) = s_0 + s_m * cos(phi + 2 * M_PI * u_h);
            prediction(2) = s_0 + s_m * cos(phi + 2 * M_PI * u_v);
            prediction(3) = s_0 + s_m * cos(phi + 2 * M_PI * u_h + 2 * M_PI * u_v);

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
            ublas_vec v = expected - prediction;
            boost::numeric::ublas::matrix<double> H(4, 5);
            for (int i = 0; i < 4; ++i)
                H(i, 0) = 1.0;
            H(0, 1) = cos(phi);
            H(1, 1) = cos(phi + 2.0 * M_PI * u_h);
            H(2, 1) = cos(phi + 2.0 * M_PI * u_v);
            H(3, 1) = cos(phi + 2.0 * M_PI * u_h + 2.0 * M_PI * u_v);
            H(0, 2) = -s_m * sin(phi);
            H(1, 2) = -s_m * sin(phi + 2.0 * M_PI * u_h);
            H(2, 2) = -s_m * sin(phi + 2.0 * M_PI * u_v);
            H(3, 2) = -s_m * sin(phi + 2.0 * M_PI * u_h + 2.0 * M_PI * u_v);
            H(0, 3) = 0.0;
            H(1, 3) = -2.0 * M_PI * s_m * sin(phi + 2.0 * M_PI * u_h);
            H(2, 3) = 0.0;
            H(3, 3) = -2.0 * M_PI * s_m * sin(phi + 2.0 * M_PI * u_h + 2.0 * M_PI * u_v);
            H(0, 4) = 0.0;
            H(1, 4) = 0.0;
            H(2, 4) = -2.0 * M_PI * s_m * sin(phi + 2.0 * M_PI * u_v);
            H(3, 4) = -2.0 * M_PI * s_m * sin(phi + 2.0 * M_PI * u_h + 2.0 * M_PI * u_v);

            using boost::numeric::ublas::prod;
            using boost::numeric::ublas::trans;
            matrix_t R_pr = estimateRpr(std::integral_constant<bool, kAutoFilter>{}, state,
                                        {{h, w}, {h, w}}, A, B, kAutoFilter ? R_w_ : R_pr_);
            boost::numeric::ublas::matrix<double> left_mult = prod(R_pr_, trans(H));
            auto right_mult = inverse(boost::numeric::ublas::matrix<double>(prod(H, left_mult) + R_n_));
            boost::numeric::ublas::matrix<double> P = prod(left_mult, right_mult);
            state.cur_signal_param[w] = state.cur_signal_param[w] + prod(P, v);
            const auto& corrected = state.cur_signal_param[w];
            raw_data[h * width + w] = corrected(1);
            swap(state.cur_signal_param, state.prev_signal_param);
        }
    }

    return ScanData(width, height, std::move(raw_data));
}

template ScanData KalmanFilter::applyKalmanFiltering<true>(const ScanData& source_data) const;
template ScanData KalmanFilter::applyKalmanFiltering<false>(const ScanData& source_data) const;

void KalmanFilter::stop() const {
    emit finished({QMessageBox::Information, tr("Done"), tr("Estimation was cancelled")});
}

void KalmanFilter::filterImpl(QString input_csv, QString output_csv, QString output_png) {
    try
    {
        ScanData scan_data(0, 0, {});
        emit reportProgress(tr("Reading input..."), 0.0);
        scan_data = ScanData::fromFile(input_csv);
        emit reportProgress(kFilteringStatus, kReadWriteFrac);
        if (interrupted_) {
            stop();
            return;
        }
        if (auto_r_pr_) {
            scan_data = applyKalmanFiltering<true>(scan_data);
        } else {
            scan_data = applyKalmanFiltering<false>(scan_data);
        }
        if (interrupted_) {
            stop();
            return;
        }
        emit reportProgress(tr("Saving %1...").arg(output_csv), 1.0 - kReadWriteFrac);
        scan_data.saveCsv(output_csv);
        emit reportProgress(tr("Saving %1...").arg(output_png), 1.0 - kReadWriteFrac / 2.0);
        scan_data.savePng(output_png, Qt::black, Qt::white);
    }
    catch (const std::exception& e)
    {
        emit finished({QMessageBox::Critical,
                       tr("Input error"),
                       tr("Error happened on input file reading: %1").arg(e.what())});
        return;
    }
    emit finished({QMessageBox::Information,
                   tr("Done"),
                   tr("Calculations finished")});
    emit reportProgress("Done", 1.0);
}

}  // namespace oct
