#pragma once

#include <QColor>
#include <QImage>

#include <vector>

std::vector<uint8_t> rawDataToRgb(const std::vector<double>& raw_data,
                                  QColor lower, QColor upper);
