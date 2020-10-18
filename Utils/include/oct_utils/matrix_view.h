#pragma once

#include <type_traits>
#include <utility>

namespace oct {

template <class RandomAccessContainer>
class MatrixView {
 public:
    MatrixView() = default;
    MatrixView(RandomAccessContainer& container, size_t stride)
        : container_(&container), stride_(stride) {}

    auto operator()(size_t i, size_t j) -> decltype(auto)
    {
        return (*container_)[i * stride_ + j];
    }
    auto operator()(size_t i, size_t j) const -> decltype(auto)
    {
        return (*container_)[i * stride_ + j];
    }

 private:
    RandomAccessContainer* container_ = nullptr;
    size_t stride_ = 0;
};

}  // namespace oct
