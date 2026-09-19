#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace vectoramp::labs {

enum class Isa {
  scalar,
  avx2,
  avx512,
};

[[nodiscard]] bool isa_supported(Isa isa) noexcept;
[[nodiscard]] std::string_view isa_name(Isa isa) noexcept;

[[nodiscard]] float l2_squared_scalar(
    const float* lhs,
    const float* rhs,
    std::size_t dimensions) noexcept;

[[nodiscard]] float l2_squared(
    const float* lhs,
    const float* rhs,
    std::size_t dimensions,
    Isa isa) noexcept;

// The distance tables are laid out as `subquantizers` consecutive rows, each
// containing 256 f32 entries. `codes[m]` selects one entry from row m.
[[nodiscard]] float pq_adc_scalar(
    const std::uint8_t* codes,
    const float* distance_tables,
    std::size_t subquantizers) noexcept;

[[nodiscard]] float pq_adc(
    const std::uint8_t* codes,
    const float* distance_tables,
    std::size_t subquantizers,
    Isa isa) noexcept;

}  // namespace vectoramp::labs

