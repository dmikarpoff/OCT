#pragma once

#include "layer_property.h"

#include <QObject>

#include <atomic>
#include <vector>

class IterativeTissueSolver : public QObject
{
    Q_OBJECT
public:
    IterativeTissueSolver(QObject* parent, const std::vector<LayerProperty>& lp,
                          size_t width, size_t height);
    std::vector<double> extractData()
    {
        return std::move(data_);
    }

public slots:
   void estimate();
   void stop();

signals:
   void sigStep(QString stage, int step_num, double step_len);
   void finished();

 private:
    void generateBounds();
    void estimateLayer(size_t idx);
    void mergeSolution();

    struct IdxValue
    {
        IdxValue(size_t i, double v): idx(i), value(v) {}
        size_t idx;
        double value;
    };
    struct ZSegment
    {
        int from;
        int to;
    };

    struct LinearIndexer {
        explicit LinearIndexer(uint32_t s) : stride(s) {}

        uint64_t index(uint32_t i, uint32_t j)
        {
            return i * stride + j;
        }

        uint32_t stride;
    };

   std::vector<LayerProperty> layer_prop_;
   uint32_t width_ = 0;
   uint32_t height_ = 0;
   std::vector<std::vector<int>> z_bounds_;
   std::vector<std::vector<double>> separate_layer_data_;
   std::vector<double> data_;
   std::atomic<bool> stopped_;
   std::vector<ZSegment> layer_ranges_;
};
