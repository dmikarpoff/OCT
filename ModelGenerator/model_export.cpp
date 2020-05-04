#include "model_export.h"

#include "image_utils.h"

#include <QFile>
#include <QTextStream>

void export_csv(QString file_name, size_t width, size_t height, const std::vector<double>& data)
{
    QFile file(file_name);
    if (!file.open(QIODevice::Text | QIODevice::WriteOnly))
        return;

    QTextStream out(&file);
    out << "width, " << width << ", height, " << height << endl;
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);
    for (size_t i = 0; i < height; ++i)
    {
        for (size_t j = 0; j < width; ++j)
        {
            if (j != 0)
            {
                out << ", ";
            }
            out << data[i * width + j];
        }
        out << endl;
    }
}

void export_png(QString file_name, size_t width, size_t height, const std::vector<double>& data,
                QColor from, QColor to)
{
    auto img_data = rawDataToRgb(data, from, to);
    QImage img(img_data.data(), static_cast<int>(width), static_cast<int>(height),
               QImage::Format_ARGB32);
    img.save(file_name);
}
