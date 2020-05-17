#pragma once

#include <oct_utils/export.h>

#include <QString>

#include <vector>

namespace oct {

class OCT_UTILS_API ScanData
{
 public:
    static ScanData fromFile(QString file_name);

    ScanData(size_t width, size_t height, std::vector<double> raw_data);
    void saveToFile(QString file_name);

 private:
    size_t width_;
    size_t height_;
    std::vector<double> raw_data_;
};

}  // namespace oct
