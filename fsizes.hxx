#pragma once

#include <concepts>

namespace sizes {

constexpr std::size_t binary_order_multiplier = 1024;

template<std::unsigned_integral T = std::size_t>
constexpr T next_order(T old_order) {
    return old_order * binary_order_multiplier;
}

template<std::unsigned_integral T = std::size_t>
constexpr T kilobytes(T nb) {
    return next_order(nb);
}

template<std::unsigned_integral T = std::size_t>
constexpr T megabytes(T nb) {
    return next_order(kilobytes(nb));
}

template<std::unsigned_integral T = std::size_t>
constexpr T gigabytes(T nb) {
    return next_order(megabytes(nb));
}

template<std::unsigned_integral T = std::size_t>
constexpr T terabytes(T nb) {
    return next_order(gigabytes(nb));
}

}