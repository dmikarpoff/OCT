#pragma once

#include <QColor>
#include <QString>

#include <vector>


void export_csv(QString file_name, size_t width, size_t height, const std::vector<double>& data);
void export_png(QString file_name, size_t width, size_t height, const std::vector<double>& data,
                QColor from, QColor to);

inline void export_model(QString file_name, size_t width, size_t height,
                         const std::vector<double>& data, QColor lower,
                         QColor higher)
{
    if (data.size() != width * height)
    {
        return;
    }
    if (file_name.endsWith(".csv", Qt::CaseInsensitive))
    {
        export_csv(file_name, width, height, data);
        return;
    }
    if (file_name.endsWith(".png", Qt::CaseInsensitive))
    {
        export_png(file_name, width, height, data, lower, higher);
        return;
    }
}
