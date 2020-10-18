#pragma once

#include "oct_utils/export.h"

#include <QColor>
#include <QImage>

#include <vector>

namespace oct
{

std::vector<uint8_t> OCT_UTILS_API rawDataToRgb(const double* raw_data, size_t data_size,
                                                QColor lower, QColor upper);

}  // namespace oct
