#include "iterativetissuesolver.h"

#include <boost/circular_buffer.hpp>

#include <cassert>
#include <vector>
#include <random>

namespace
{

double distance(const std::vector<double>& lhs, const std::vector<double>& rhs)
{
    assert(lhs.size() == rhs.size());
    double res = 0.0;
    for (size_t i = 0; i < lhs.size(); ++i)
    {
        res = std::max(res, std::abs(lhs[i] - rhs[i]));
    }
    return res;
}

}

IterativeTissueSolver::IterativeTissueSolver(QObject* parent,
                            const std::vector<LayerProperty>& lp, size_t width, size_t height)
    : QObject(parent)
    , layer_prop_(lp)
    , width_(static_cast<uint32_t>(width))
    , height_(static_cast<uint32_t>(height))
    , z_bounds_(lp.size() - 1, std::vector<int>(width_, 0))
{
}

void IterativeTissueSolver::estimate()
{
    stopped_ = false;
    data_.clear();
    separate_layer_data_.clear();
    generateBounds();
    for (size_t i = 0; i < layer_prop_.size() && !stopped_; ++i)
    {
        separate_layer_data_.emplace_back();
        estimateLayer(i);
    }
    if (!stopped_)
    {
        mergeSolution();
    }
    emit finished();
    stopped_ = true;
}

void IterativeTissueSolver::stop()
{
    stopped_ = true;
}

void IterativeTissueSolver::generateBounds()
{
    emit sigStep("Generate layer bounds", 0, std::numeric_limits<double>::quiet_NaN());
    layer_ranges_.resize(layer_prop_.size());
    layer_ranges_.front().from = 0;
    layer_ranges_.back().to = height_ - 1;
    for (size_t i = 0; i < z_bounds_.size(); ++i)
    {
        emit sigStep("Generate layer bounds", static_cast<int>(i),
                     std::numeric_limits<double>::quiet_NaN());
        const auto& cur_description = layer_prop_[i];
        int lower = i > 0 ? layer_prop_[i - 1].max_z + 1 : 1;
        int delta = layer_prop_[i].max_z - lower;
        layer_ranges_[i + 1].from = lower - 1;
        layer_ranges_[i].to = lower + delta - 1;
        std::vector<double> raw_bound(width_, 0.0);
        raw_bound[0] = 0.0;
        std::mt19937 generator(std::random_device{}());
        std::normal_distribution<> noise(0.0, cur_description.sigma);
        double max_seq = raw_bound[0];
        double min_seq = raw_bound[0];
        for (size_t j = 1; j < raw_bound.size(); ++j)
        {
            double prev = raw_bound[j - 1];
            raw_bound[j] = - cur_description.alpha * prev * std::exp(-cur_description.alpha) +
                           cur_description.alpha * noise(generator);
            if (raw_bound[j] > max_seq)
            {
                max_seq = raw_bound[j];
            }
            if (raw_bound[j] < min_seq)
            {
                min_seq = raw_bound[j];
            }
        }
        for (size_t j = 0; j < raw_bound.size(); ++j)
        {
            z_bounds_[i][j] = static_cast<int>(std::round(lower + delta *
                                (raw_bound[j] - min_seq) / (max_seq - min_seq)));
        }
        auto w = layer_prop_[i].border_aver_window;
        int sum = w * z_bounds_[i][0];
        boost::circular_buffer<int> values(w, z_bounds_[i][0]);
        for (size_t j = 0; j < raw_bound.size(); ++j)
        {
            sum -= values.front();
            sum += z_bounds_[i][j];
            values.push_back(z_bounds_[i][j]);
            z_bounds_[i][j] = sum / w;
        }
    }
}

void IterativeTissueSolver::estimateLayer(size_t idx)
{
    QString stage = QString::fromStdString("Estimate layer %1").arg(idx);
    size_t width = width_;
    size_t height = layer_ranges_[idx].to - layer_ranges_[idx].from + 1;
    std::vector<std::vector<IdxValue>> A(width * height);
    const auto& layer_prop = layer_prop_[idx];
    LinearIndexer index(static_cast<uint32_t>(width));
    for (uint32_t i = 0; i < height; ++i)
    {
        for (uint32_t j = 0; j < width; ++j)
        {
            uint64_t idx = index.index(i, j);
            if (i > 0)
            {
                A[idx].emplace_back(index.index(i - 1, j), layer_prop.a);
            }
            if (j > 0)
            {
                A[idx].emplace_back(index.index(i, j - 1), layer_prop.d);
            }
            if (j + 1 < width)
            {
                A[idx].emplace_back(index.index(i, j + 1), layer_prop.c);
            }
            if (i + 1 < height)
            {
                A[idx].emplace_back(index.index(i + 1, j), layer_prop.b);
            }
        }
    }
    std::vector<double> cur(width * height, 0.0);
    std::vector<double> next(width * height, 0.0);
    std::vector<double> w(width * height);

    std::mt19937 generator(std::random_device{}());
    std::normal_distribution<> noise;
    for (size_t i = 0; i < w.size(); ++i)
        w[i] = noise(generator);

    size_t iter = 0;
    double d = 0.0;
    do
    {
        if (stopped_)
        {
            return;
        }
        std::swap(cur, next);
        for (size_t i = 0; i < cur.size() && !stopped_; ++i)
        {
            double prod = 0.0;
            for (const auto& i_v : A[i])
            {
                prod += i_v.value * (i_v.idx < i ? next[i_v.idx] : cur[i_v.idx]);
            }
            next[i] = (1.0 - layer_prop.alpha) * prod + layer_prop.alpha * w[i];
        }
        iter++;
        d = distance(cur, next);
        emit sigStep(stage, static_cast<int>(iter), d);
    } while (d > 1e-6);
    if (!stopped_)
    {
        separate_layer_data_.back().reserve(next.size());
        double delta = 1.0;
        for (auto v : next)
        {
            separate_layer_data_.back().push_back(std::min(1.0, std::max(0.0, (v + delta / 2.0) / delta)));
        }
    }
}

void IterativeTissueSolver::mergeSolution()
{
    emit sigStep("Merging final solution", 0, std::numeric_limits<double>::quiet_NaN());
    data_.resize(width_ * height_, 0.0);
    LinearIndexer index(width_);
    for (auto w = 0u; w < width_; ++w)
    {
        auto layer_num = 0u;
        for (auto h = 0u; h < height_; ++h)
        {
            if (z_bounds_.size() != layer_num && z_bounds_[layer_num][w] == static_cast<int>(h))
            {
                ++layer_num;
            }
            uint32_t rel_pos = h - layer_ranges_[layer_num].from;
            data_[index.index(h, w)] =
                separate_layer_data_[layer_num][index.index(rel_pos, w)];
        }
    }
}
