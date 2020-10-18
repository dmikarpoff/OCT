#include "oct_utils/scan_data.h"

#include "oct_utils/image_utils.h"
#include "scan_data_loader.h"

#include <QFile>
#include <QImage>
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
    , data_view_(raw_data_, width_)
{
}

ScanData::ScanData(ScanData&& other)
    : width_(other.width_)
    , height_(other.height_)
    , raw_data_(std::move(other.raw_data_))
    , data_view_(raw_data_, width_)
{
}

ScanData& ScanData::operator=(ScanData&& other)
{
    width_ = other.width_;
    height_ = other.height_;
    raw_data_ = other.raw_data_;
    data_view_ = MatrixView<std::vector<double>>(raw_data_, width_);
    return *this;
}

void ScanData::saveCsv(QString file_name) const
{
    QFile file(file_name);
    if (!file.open(QIODevice::Text | QIODevice::WriteOnly))
        return;

    QTextStream out(&file);
    out << "width, " << width_ << ", height, " << height_ << endl;
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);
    for (size_t i = 0; i < height_; ++i)
    {
        for (size_t j = 0; j < width_; ++j)
        {
            if (j != 0)
            {
                out << ", ";
            }
            out << raw_data_[i * width_ + j];
        }
        out << endl;
    }
}

void ScanData::savePng(QString file_name, QColor lower, QColor upper) const
{
    auto img_data = rawDataToRgb(raw_data_.data(), raw_data_.size(), lower, upper);
    QImage img(img_data.data(), static_cast<int>(width_), static_cast<int>(height_),
               QImage::Format_ARGB32);
    img.save(file_name);
}

}  // namespace oct
