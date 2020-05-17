#pragma once

#include "oct_utils/scan_data.h"

namespace oct {

ScanData applyKalmanFiltering(const ScanData& source_data);

}  // namespace oct
