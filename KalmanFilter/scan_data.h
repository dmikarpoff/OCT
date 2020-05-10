#pragma once

#include <QString>

#include <vector>

class ScanData
{
 public:
    static ScanData fromFile(QString file_name);

    ScanData(size_t width, size_t height, std::vector<double> raw_data);

 private:
    size_t width_;
    size_t height_;
    std::vector<double> raw_data_;
};
