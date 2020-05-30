#include "oct_utils/scan_data.h"

#include "scan_data_loader.h"

#include <QFile>
#include <QTextStream>

namespace oct {

ScanData ScanData::fromFile(QString file_name)
{
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open file with name " + file_name.toStdString());
    }
    size_t width = 0;
    size_t height = 0;
    std::vector<double> raw_data = helpers::loadFromCsv(file, &width, &height);
    return ScanData(width, height, std::move(raw_data));
}

ScanData::ScanData(size_t width, size_t height, std::vector<double> raw_data)
    : width_(width)
    , height_(height)
    , raw_data_(std::move(raw_data))
    , data_view_(new MatrixView<std::vector<double>>(raw_data_, width_)){}

void ScanData::saveToFile(QString file_name) {

}

}  // namespace oct
