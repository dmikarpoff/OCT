#include "scan_data.h"

#include <QFile>

ScanData ScanData::fromFile(QString file_name)
{
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open file with name " + file_name.toStdString());
    }
    return ScanData{0, 0, {}};
}

ScanData::ScanData(size_t width, size_t height, std::vector<double> raw_data)
    : width_(width)
    , height_(height)
    , raw_data_(raw_data) {}
