#pragma once

#include <oct_utils/export.h>
#include <oct_utils/matrix_view.h>

#include <QString>

#include <memory>
#include <vector>

namespace oct {

class OCT_UTILS_API ScanData
{
 public:
    static ScanData fromFile(QString file_name);

    ScanData(size_t width, size_t height, std::vector<double> raw_data);

    size_t width() const {
        return width_;
    }
    size_t height() const {
        return height_;
    }

    double operator()(size_t i, size_t j)
    {
        return (*data_view_)(i, j);
    }

    double operator()(size_t i, size_t j) const
    {
        return (*data_view_)(i, j);
    }

    void saveToFile(QString file_name);

 private:
    size_t width_;
    size_t height_;
    std::vector<double> raw_data_;
    std::unique_ptr<MatrixView<std::vector<double>>> data_view_;
};

}  // namespace oct
