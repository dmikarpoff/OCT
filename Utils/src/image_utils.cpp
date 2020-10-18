#include "oct_utils/image_utils.h"

namespace oct
{

std::vector<uint8_t> rawDataToRgb(const double* raw_data, size_t data_size,
                                  QColor lower, QColor upper)
{
    std::vector<uint8_t> img_data;
    img_data.reserve(data_size * 4);
    uint8_t lower_arr[4] = {static_cast<uint8_t>(lower.blue()), static_cast<uint8_t>(lower.green()),
                           static_cast<uint8_t>(lower.red()), static_cast<uint8_t>(lower.alpha())};
    uint8_t upper_arr[4] = {static_cast<uint8_t>(upper.blue()), static_cast<uint8_t>(upper.green()),
                           static_cast<uint8_t>(upper.red()), static_cast<uint8_t>(upper.alpha())};
    for (auto i = 0u; i < data_size; ++i)
    {
        std::transform(std::begin(lower_arr), std::end(lower_arr), std::begin(upper_arr),
                       std::back_inserter(img_data),
                       [val=raw_data[i]](auto l, auto u)
                        {
                            return static_cast<uint8_t>((1.0 - val) * l + val * u);
                        });
    }
    return img_data;
}

}  // namespace oct
