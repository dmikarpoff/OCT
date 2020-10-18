#pragma once

#include <oct_utils/export.h>
#include <oct_utils/matrix_view.h>

#include <QColor>
#include <QString>

#include <memory>
#include <vector>

namespace oct {

class OCT_UTILS_API ScanData
{
 public:
    static ScanData fromFile(QString file_name);

    ScanData() = default;
    ScanData(size_t width, size_t height, std::vector<double> raw_data);
    ~ScanData() = default;

    ScanData(const ScanData&) = delete;
    ScanData& operator=(const ScanData&) = delete;

    ScanData(ScanData&& other);
    ScanData& operator=(ScanData&& other);

    size_t width() const {
        return width_;
    }
    size_t height() const {
        return height_;
    }
    bool empty() const {
        return width_ == 0 || height_ == 0;
    }
    const double* data() const {
        return raw_data_.data();
    }

    double operator()(size_t i, size_t j)
    {
        return data_view_(i, j);
    }

    double operator()(size_t i, size_t j) const
    {
        return data_view_(i, j);
    }

    void saveCsv(QString file_name) const;
    void savePng(QString file_name, QColor lower, QColor upper) const;

 private:
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<double> raw_data_;
    MatrixView<std::vector<double>> data_view_;
};

}  // namespace oct
