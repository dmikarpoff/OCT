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
    ScanData res{0, 0, {}};
    res.raw_data_ = helpers::loadFromCsv(file, &res.width_, &res.height_);
    return res;
}

ScanData::ScanData(size_t width, size_t height, std::vector<double> raw_data)
    : width_(width)
    , height_(height)
    , raw_data_(raw_data) {}

void ScanData::saveToFile(QString file_name) {

}

}  // namespace oct
